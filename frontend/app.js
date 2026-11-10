const $ = (id) => document.getElementById(id);
const GROUPS = ['A+', 'A-', 'B+', 'B-', 'AB+', 'AB-', 'O+', 'O-'];
const MONTHS = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
const GENDER = {M: 'Male', F: 'Female', O: 'Other'};
const PAGE_TITLES = {dashboard: 'Dashboard', donors: 'Donors', add: 'Register donor', inventory: 'Inventory'};

let donors = [];
let inventory = {stocks: [], total_units: 0};
let noticeTimer;

function escapeHtml(value) {
  return String(value ?? '').replace(/[&<>"']/g, c => ({'&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'}[c]));
}

function formatCount(value) {
  return new Intl.NumberFormat('en-US').format(Number(value) || 0);
}

function initials(name) {
  const parts = String(name || '').trim().split(/\s+/).filter(Boolean);
  if (!parts.length) return '—';
  if (parts.length === 1) return parts[0].slice(0, 2).toUpperCase();
  return (parts[0][0] + parts[parts.length - 1][0]).toUpperCase();
}

function lastDonationLabel(donor) {
  if (!donor.has_donated_before) return 'No donation on file';
  const date = donor.last_donation || {};
  const day = Number(date.day), month = Number(date.month), year = Number(date.year);
  if (!day || !month || !year || month < 1 || month > 12) return 'No donation on file';
  return `${String(day).padStart(2, '0')} ${MONTHS[month - 1]} ${year}`;
}

function notice(message, error = false) {
  const box = $('notice');
  $('notice-text').textContent = message;
  box.className = error ? 'notice error' : 'notice';
  box.hidden = false;
  window.clearTimeout(noticeTimer);
  noticeTimer = window.setTimeout(() => { box.hidden = true; }, 4500);
}

async function api(url, options = {}) {
  const response = await fetch(url, {headers: {'Content-Type': 'application/json'}, ...options});
  const data = await response.json();
  if (!response.ok || data.ok === false) throw new Error(data.error || 'Request failed');
  return data;
}

function showPage(page) {
  document.querySelectorAll('.page').forEach(el => el.classList.toggle('active', el.id === page));
  document.querySelectorAll('.nav').forEach(el => el.classList.toggle('active', el.dataset.page === page));
  $('page-title').textContent = PAGE_TITLES[page] || 'Dashboard';
  if (page === 'add' && !$('edit-id').value) resetDonorForm();
  window.scrollTo(0, 0);
}

function emptyState(title, copy, action) {
  return `<div class="empty-state">
    <span class="empty-mark"><svg class="icon" aria-hidden="true"><use href="#i-droplet"/></svg></span>
    <div><strong>${escapeHtml(title)}</strong><p>${escapeHtml(copy)}</p>${action || ''}</div>
  </div>`;
}

function renderRack(stocks) {
  const target = $('dashboard-stocks');
  if (!stocks.length) {
    target.innerHTML = '<p class="supply-meta">No groups reported by the local core.</p>';
    return;
  }
  const max = Math.max(1, ...stocks.map(s => Number(s.units) || 0));
  target.innerHTML = stocks.map(s => {
    const units = Number(s.units) || 0;
    const pct = units ? Math.max(8, Math.round((units / max) * 100)) : 0;
    return `<div class="vial-group ${units ? 'has-stock' : ''}">
      <div class="vial" role="img" aria-label="${escapeHtml(s.blood_group)}: ${units} ${units === 1 ? 'unit' : 'units'}">
        <span class="vial-fill" style="--fill:${pct}%"></span>
      </div>
      <div class="vial-meta"><span class="vial-type">${escapeHtml(s.blood_group)}</span><span class="vial-units">${units}</span></div>
    </div>`;
  }).join('');
}

function renderStockGrid(stocks) {
  const target = $('inventory-stocks');
  if (!stocks.length) {
    target.innerHTML = '<p class="supply-meta">No groups reported by the local core.</p>';
    return;
  }
  const max = Math.max(1, ...stocks.map(s => Number(s.units) || 0));
  target.innerHTML = stocks.map(s => {
    const units = Number(s.units) || 0;
    const pct = units ? Math.max(4, Math.round((units / max) * 100)) : 0;
    return `<article class="stock-card ${units ? '' : 'is-empty'}">
      <div class="stock-head">
        <span class="blood-tag">${escapeHtml(s.blood_group)}</span>
        <span class="stock-state ${units ? 'in' : ''}">${units ? 'In stock' : 'Empty'}</span>
      </div>
      <div class="stock-value">${units}<small>units</small></div>
      <div class="meter" aria-hidden="true"><span style="--pct:${pct}%"></span></div>
    </article>`;
  }).join('');
}

function renderSupplySummary(stocks) {
  const total = stocks.reduce((sum, s) => sum + (Number(s.units) || 0), 0);
  const stocked = stocks.filter(s => Number(s.units) > 0);
  let summary;
  if (!stocks.length) summary = 'Waiting on the local core';
  else if (!stocked.length) summary = 'The shelf is empty';
  else if (stocked.length === 1) summary = `${stocked[0].blood_group} is the only group on the shelf`;
  else if (stocked.length === stocks.length) summary = 'Every group has units on the shelf';
  else summary = `${stocked.length} groups are on the shelf`;

  $('supply-summary').textContent = summary;
  $('supply-meta').textContent = stocks.length
    ? `${formatCount(total)} ${total === 1 ? 'unit' : 'units'} · ${stocked.length} of ${stocks.length} groups stocked`
    : '—';
}

function renderRecent() {
  const target = $('recent-donors');
  const recent = donors.slice(-5).reverse();
  if (!recent.length) {
    target.innerHTML = emptyState('No donors registered', 'Add the first record to start the registry.');
    return;
  }
  target.innerHTML = recent.map(d => `<div class="recent-row">
    <span class="avatar" aria-hidden="true">${escapeHtml(initials(d.name))}</span>
    <div>
      <span class="recent-name">${escapeHtml(d.name)}</span>
      <span class="recent-meta"><span>ID ${d.id}</span><span class="meta-dot"></span><span>${escapeHtml(lastDonationLabel(d))}</span></span>
    </div>
    <span class="blood-tag">${escapeHtml(d.blood_group)}</span>
  </div>`).join('');
}

function renderWatch(stocks) {
  const target = $('stock-watch');
  const empty = stocks.filter(s => !Number(s.units));
  if (!stocks.length) {
    target.innerHTML = '<p class="watch-more">No stock report available.</p>';
    return;
  }
  if (!empty.length) {
    target.innerHTML = '<div class="watch-clear"><span class="status-dot" aria-hidden="true"></span><span>All eight groups hold units. Nothing to restock.</span></div>';
    return;
  }
  const shown = empty.slice(0, 4);
  const remaining = empty.length - shown.length;
  target.innerHTML = shown.map(s => `<div class="watch-row">
    <span class="watch-flag" aria-hidden="true"></span>
    <span class="blood-tag">${escapeHtml(s.blood_group)}</span>
    <span class="watch-units">0 units</span>
  </div>`).join('') + (remaining > 0 ? `<p class="watch-more">+${remaining} more ${remaining === 1 ? 'group' : 'groups'} at zero</p>` : '');
}

function renderDonors() {
  const query = ($('search').value || '').trim().toLowerCase();
  const filtered = donors.filter(d => [d.id, d.name, d.blood_group, d.phone, GENDER[d.gender]]
    .some(v => String(v ?? '').toLowerCase().includes(query)));

  $('donor-result-count').textContent = query
    ? `${filtered.length} of ${donors.length} ${donors.length === 1 ? 'donor' : 'donors'}`
    : `${donors.length} ${donors.length === 1 ? 'donor' : 'donors'}`;

  if (!donors.length) {
    $('donor-rows').innerHTML = `<tr><td colspan="7"><div class="table-empty">
      <svg class="icon" aria-hidden="true"><use href="#i-droplet"/></svg>
      <strong>No donors registered</strong><span>The registry is empty. Add the first donor record to begin.</span>
      <button type="button" class="link-btn" data-go="add">Register donor
        <svg class="icon" aria-hidden="true"><use href="#i-arrow-right"/></svg>
      </button>
    </div></td></tr>`;
    return;
  }

  $('donor-rows').innerHTML = filtered.map(d => `<tr>
    <td><div class="donor-cell"><span class="avatar" aria-hidden="true">${escapeHtml(initials(d.name))}</span><div><span class="donor-name">${escapeHtml(d.name)}</span><span class="donor-id">ID ${d.id}</span></div></div></td>
    <td>${Number(d.age)}</td>
    <td>${escapeHtml(GENDER[d.gender] || d.gender || '—')}</td>
    <td><span class="blood-tag">${escapeHtml(d.blood_group)}</span></td>
    <td>${Number(d.weight).toFixed(1)} kg</td>
    <td>${d.phone ? escapeHtml(d.phone) : '—'}</td>
    <td><div class="actions">
      <button type="button" class="icon-btn small" data-edit="${d.id}" aria-label="Edit ${escapeHtml(d.name)}" title="Edit donor"><svg class="icon" aria-hidden="true"><use href="#i-pencil"/></svg></button>
      <button type="button" class="icon-btn small danger" data-delete="${d.id}" aria-label="Delete ${escapeHtml(d.name)}" title="Delete donor"><svg class="icon" aria-hidden="true"><use href="#i-trash"/></svg></button>
    </div></td>
  </tr>`).join('') || `<tr><td colspan="7"><div class="table-empty">
      <svg class="icon" aria-hidden="true"><use href="#i-search"/></svg>
      <strong>No matching donors</strong><span>Try a different name, ID or blood group.</span>
    </div></td></tr>`;
}

function updateSlip() {
  const group = $('blood_group').value;
  const name = $('name').value.trim();
  const age = $('age').value;
  const weight = $('weight').value;
  const id = $('edit-id').value;
  $('slip-group').textContent = group || 'Not set';
  $('slip-group').classList.toggle('is-empty', !group);
  $('slip-name').textContent = name || 'New donor';
  $('slip-id').textContent = id ? `ID ${id}` : 'ID pending';
  $('slip-age').textContent = age ? `Age ${age}` : 'Age —';
  $('slip-weight').textContent = weight ? `${Number(weight).toFixed(1)} kg` : 'Weight —';
}

function resetDonorForm(resetFields = true) {
  if (resetFields) $('donor-form').reset();
  $('edit-id').value = '';
  $('has_donated_before').checked = false;
  $('date-fields').hidden = true;
  $('form-heading').textContent = 'Donor details';
  $('form-subtitle').textContent = 'Add a donor to the local registry';
  $('save-btn').textContent = 'Save donor';
  updateSlip();
}

function donorPayload() {
  return {
    name: $('name').value.trim(),
    age: Number($('age').value),
    gender: $('gender').value,
    weight: Number($('weight').value),
    blood_group: $('blood_group').value,
    phone: $('phone').value.trim(),
    has_donated_before: $('has_donated_before').checked,
    day: Number($('day').value || 0),
    month: Number($('month').value || 0),
    year: Number($('year').value || 0)
  };
}

async function editDonor(id) {
  try {
    const result = await api(`/api/donors/${id}`);
    const d = result.donor;
    $('edit-id').value = d.id;
    showPage('add');
    $('name').value = d.name;
    $('age').value = d.age;
    $('gender').value = d.gender;
    $('weight').value = d.weight;
    $('blood_group').value = d.blood_group;
    $('phone').value = d.phone;
    $('has_donated_before').checked = d.has_donated_before;
    $('day').value = d.last_donation.day || '';
    $('month').value = d.last_donation.month || '';
    $('year').value = d.last_donation.year || '';
    $('date-fields').hidden = !d.has_donated_before;
    $('form-heading').textContent = `Edit record #${d.id}`;
    $('form-subtitle').textContent = 'Update the record and save your changes';
    $('save-btn').textContent = 'Update donor';
    $('page-title').textContent = `Donor #${d.id}`;
    updateSlip();
  } catch (err) {
    notice(err.message, true);
  }
}

async function refreshAll() {
  try {
    const [donorData, stockData] = await Promise.all([api('/api/donors'), api('/api/inventory')]);
    donors = donorData.donors || [];
    inventory = stockData;
    const stocks = stockData.stocks || [];
    const total = Number(stockData.total_units) || 0;
    const stocked = stocks.filter(s => Number(s.units) > 0).length;

    $('donor-count').textContent = formatCount(donors.length);
    $('unit-count').textContent = formatCount(total);
    $('group-count').textContent = formatCount(stocked);
    $('inventory-total').textContent = `${formatCount(total)} ${total === 1 ? 'unit' : 'units'}`;
    $('recent-count').textContent = `${donors.length} ${donors.length === 1 ? 'record' : 'records'} in the registry`;

    renderRack(stocks);
    renderSupplySummary(stocks);
    renderStockGrid(stocks);
    renderRecent();
    renderWatch(stocks);
    renderDonors();
  } catch (err) {
    notice(err.message, true);
  }
}

function fillGroupSelects() {
  ['restock-group', 'dispense-group'].forEach(id => {
    $(id).innerHTML = GROUPS.map(g => `<option value="${g}">${g}</option>`).join('');
  });
}

function inventoryAction(formId, endpoint, groupId, unitsId) {
  $(formId).addEventListener('submit', async event => {
    event.preventDefault();
    const button = event.target.querySelector('button[type="submit"]');
    button.disabled = true;
    try {
      await api(endpoint, {method: 'POST', body: JSON.stringify({blood_group: $(groupId).value, units: Number($(unitsId).value)})});
      notice('Inventory updated');
      event.target.reset();
      await refreshAll();
    } catch (err) {
      notice(err.message, true);
    } finally {
      button.disabled = false;
    }
  });
}

/* ---------------- events ---------------- */

document.querySelectorAll('.nav').forEach(btn => btn.addEventListener('click', () => {
  if (btn.dataset.page === 'add') resetDonorForm();
  showPage(btn.dataset.page);
}));

document.addEventListener('click', async event => {
  const go = event.target.closest('[data-go]');
  if (go) {
    if (go.dataset.go === 'add') resetDonorForm();
    showPage(go.dataset.go);
    return;
  }
  const edit = event.target.closest('[data-edit]');
  if (edit) { editDonor(edit.dataset.edit); return; }
  const del = event.target.closest('[data-delete]');
  if (del && confirm(`Delete donor #${del.dataset.delete}?`)) {
    try {
      await api(`/api/donors/${del.dataset.delete}`, {method: 'DELETE'});
      notice('Donor deleted');
      await refreshAll();
    } catch (err) {
      notice(err.message, true);
    }
  }
});

$('refresh').addEventListener('click', async () => {
  const button = $('refresh');
  button.classList.add('is-loading');
  await refreshAll();
  window.setTimeout(() => button.classList.remove('is-loading'), 400);
});

$('search').addEventListener('input', renderDonors);
$('search').addEventListener('keydown', event => {
  if (event.key === 'Escape') {
    $('search').value = '';
    renderDonors();
    $('search').blur();
  }
});

$('clear-btn').addEventListener('click', () => resetDonorForm());
$('has_donated_before').addEventListener('change', event => {
  $('date-fields').hidden = !event.target.checked;
});
['name', 'age', 'weight', 'blood_group'].forEach(id => {
  $(id).addEventListener('input', updateSlip);
  $(id).addEventListener('change', updateSlip);
});

$('donor-form').addEventListener('submit', async event => {
  event.preventDefault();
  const id = $('edit-id').value;
  const payload = donorPayload();
  const button = $('save-btn');
  button.disabled = true;
  try {
    if (id) await api(`/api/donors/${id}`, {method: 'PUT', body: JSON.stringify(payload)});
    else await api('/api/donors', {method: 'POST', body: JSON.stringify(payload)});
    notice(id ? 'Donor record updated' : 'Donor registered');
    resetDonorForm();
    await refreshAll();
    showPage('donors');
  } catch (err) {
    notice(err.message, true);
  } finally {
    button.disabled = false;
  }
});

document.addEventListener('keydown', event => {
  const tag = (document.activeElement && document.activeElement.tagName) || '';
  if (event.key === '/' && !['INPUT', 'SELECT', 'TEXTAREA'].includes(tag)) {
    event.preventDefault();
    showPage('donors');
    $('search').focus();
  }
});

/* ---------------- boot ---------------- */

$('today').textContent = new Intl.DateTimeFormat('en-GB', {day: '2-digit', month: 'short', year: 'numeric'}).format(new Date());
fillGroupSelects();
inventoryAction('restock-form', '/api/inventory/restock', 'restock-group', 'restock-units');
inventoryAction('dispense-form', '/api/inventory/dispense', 'dispense-group', 'dispense-units');
updateSlip();
refreshAll();
