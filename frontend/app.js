const $ = (id) => document.getElementById(id);
const groups = ['A+','A-','B+','B-','AB+','AB-','O+','O-'];
let donors = [];
let inventory = null;

function notice(message, error = false) {
  const box = $('notice'); box.textContent = message; box.className = error ? 'notice error' : 'notice'; box.hidden = false;
  window.clearTimeout(notice.timer); notice.timer = window.setTimeout(() => { box.hidden = true; }, 4500);
}
async function api(url, options = {}) {
  const response = await fetch(url, {headers: {'Content-Type':'application/json'}, ...options});
  const data = await response.json();
  if (!response.ok || data.ok === false) throw new Error(data.error || 'Request failed');
  return data;
}
function showPage(page) {
  document.querySelectorAll('.page').forEach(el => el.classList.toggle('active', el.id === page));
  document.querySelectorAll('.nav').forEach(el => el.classList.toggle('active', el.dataset.page === page));
  const titles = {dashboard:'Dashboard', donors:'Donors', add:'Add donor', inventory:'Blood inventory'};
  $('page-title').textContent = titles[page] || 'Dashboard';
  if (page === 'add' && !$('edit-id').value) resetDonorForm();
}
function stockCards(target, stocks) {
  $(target).innerHTML = stocks.map(s => `<div class="stock"><strong>${escapeHtml(s.blood_group)}</strong><span>Available units</span><b>${Number(s.units)}</b></div>`).join('');
}
function escapeHtml(value) {
  return String(value ?? '').replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
}
async function refreshAll() {
  try {
    const [donorData, stockData] = await Promise.all([api('/api/donors'), api('/api/inventory')]);
    donors = donorData.donors || []; inventory = stockData;
    $('donor-count').textContent = donors.length;
    $('unit-count').textContent = stockData.total_units;
    $('inventory-total').textContent = `${stockData.total_units} units`;
    stockCards('dashboard-stocks', stockData.stocks);
    stockCards('inventory-stocks', stockData.stocks);
    $('recent-donors').innerHTML = donors.slice(-5).reverse().map(d => `<div class="recent-row">${escapeHtml(d.name)} · ${escapeHtml(d.blood_group)} · ID ${d.id}</div>`).join('') || '<p>No donors registered yet.</p>';
    renderDonors();
  } catch (err) { notice(err.message, true); }
}
function renderDonors() {
  const query = ($('search').value || '').trim().toLowerCase();
  const filtered = donors.filter(d => [d.id,d.name,d.blood_group,d.phone].some(v => String(v).toLowerCase().includes(query)));
  $('donor-rows').innerHTML = filtered.map(d => `<tr><td>${d.id}</td><td>${escapeHtml(d.name)}</td><td>${d.age}</td><td>${escapeHtml(d.gender)}</td><td><strong>${escapeHtml(d.blood_group)}</strong></td><td>${Number(d.weight).toFixed(1)} kg</td><td>${escapeHtml(d.phone)}</td><td><div class="actions"><button class="small-btn" data-edit="${d.id}">Edit</button><button class="small-btn delete" data-delete="${d.id}">Delete</button></div></td></tr>`).join('') || '<tr><td colspan="8">No matching donors.</td></tr>';
}
function resetDonorForm() {
  $('donor-form').reset(); $('edit-id').value = ''; $('form-heading').textContent = 'Register donor'; $('save-btn').textContent = 'Save donor'; $('date-fields').hidden = true;
}
function donorPayload() {
  return {
    name: $('name').value.trim(), age: Number($('age').value), gender: $('gender').value,
    weight: Number($('weight').value), blood_group: $('blood_group').value,
    phone: $('phone').value.trim(), has_donated_before: $('has_donated_before').checked,
    day: Number($('day').value || 0), month: Number($('month').value || 0), year: Number($('year').value || 0)
  };
}
async function editDonor(id) {
  try {
    const result = await api(`/api/donors/${id}`); const d = result.donor;
    $('edit-id').value = d.id; $('name').value = d.name; $('age').value = d.age;
    $('gender').value = d.gender; $('weight').value = d.weight; $('blood_group').value = d.blood_group;
    $('phone').value = d.phone; $('has_donated_before').checked = d.has_donated_before;
    $('day').value = d.last_donation.day || ''; $('month').value = d.last_donation.month || ''; $('year').value = d.last_donation.year || '';
    $('date-fields').hidden = !d.has_donated_before;
    $('form-heading').textContent = `Edit donor #${d.id}`; $('save-btn').textContent = 'Update donor'; showPage('add');
  } catch (err) { notice(err.message, true); }
}
document.querySelectorAll('.nav').forEach(btn => btn.addEventListener('click', () => showPage(btn.dataset.page)));
document.addEventListener('click', async event => {
  const go = event.target.closest('[data-go]'); if (go) showPage(go.dataset.go);
  const edit = event.target.closest('[data-edit]'); if (edit) editDonor(edit.dataset.edit);
  const del = event.target.closest('[data-delete]');
  if (del && confirm(`Delete donor #${del.dataset.delete}?`)) {
    try { await api(`/api/donors/${del.dataset.delete}`, {method:'DELETE'}); notice('Donor deleted.'); await refreshAll(); }
    catch (err) { notice(err.message, true); }
  }
});
$('refresh').addEventListener('click', refreshAll);
$('search').addEventListener('input', renderDonors);
$('has_donated_before').addEventListener('change', e => { $('date-fields').hidden = !e.target.checked; });
$('donor-form').addEventListener('submit', async event => {
  event.preventDefault(); const id = $('edit-id').value; const payload = donorPayload();
  try {
    if (id) await api(`/api/donors/${id}`, {method:'PUT', body:JSON.stringify(payload)});
    else await api('/api/donors', {method:'POST', body:JSON.stringify(payload)});
    notice(id ? 'Donor updated successfully.' : 'Donor registered successfully.'); resetDonorForm(); await refreshAll(); showPage('donors');
  } catch (err) { notice(err.message, true); }
});
function fillGroupSelects() {
  ['restock-group','dispense-group'].forEach(id => { $(id).innerHTML = groups.map(g => `<option value="${g}">${g}</option>`).join(''); });
}
async function inventoryAction(formId, endpoint, groupId, unitsId) {
  $(formId).addEventListener('submit', async event => {
    event.preventDefault();
    try {
      await api(endpoint, {method:'POST', body:JSON.stringify({blood_group:$(groupId).value, units:Number($(unitsId).value)})});
      notice('Inventory updated.'); $(formId).reset(); await refreshAll();
    } catch (err) { notice(err.message, true); }
  });
}
fillGroupSelects();
inventoryAction('restock-form','/api/inventory/restock','restock-group','restock-units');
inventoryAction('dispense-form','/api/inventory/dispense','dispense-group','dispense-units');
refreshAll();
