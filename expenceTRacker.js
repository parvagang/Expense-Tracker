// =============================================================================
//  Expense Tracker - Frontend logic
//  Sends requests to the C++ backend (/api/...) and updates the page.
// =============================================================================

const CATEGORIES = ["Food", "Travel", "Bills", "Shopping", "Entertainment", "Health", "Other"];
const API = "/api/expenses";

const $ = (id) => document.getElementById(id);
const form = $("expense-form");

let editingId = null;     // id of the expense being edited (null = adding new)
let expenses = [];        // currently displayed (filtered) transactions

// ---------- Helpers ----------
function esc(text) {
  return String(text).replace(/[&<>"']/g, (c) => ({
    "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;"
  }[c]));
}

function money(n) {
  return Number(n).toFixed(2);
}

function todayString() {
  const d = new Date();
  const mm = String(d.getMonth() + 1).padStart(2, "0");
  const dd = String(d.getDate()).padStart(2, "0");
  return `${d.getFullYear()}-${mm}-${dd}`;
}

function toast(message, isError = false) {
  const t = $("toast");
  t.textContent = message;
  t.className = "toast" + (isError ? " error" : "");
  t.hidden = false;
  clearTimeout(toast.timer);
  toast.timer = setTimeout(() => (t.hidden = true), 2500);
}

// Calls the backend and returns the parsed JSON (throws on error)
async function api(method, url, data) {
  const options = { method, headers: { "Content-Type": "application/json" } };
  if (data !== undefined) options.body = JSON.stringify(data);

  const res = await fetch(url, options);
  const json = await res.json().catch(() => ({}));
  if (!res.ok) throw new Error(json.error || "Request failed");
  return json;
}

// ---------- Loading and rendering ----------
async function loadExpenses() {
  const params = new URLSearchParams();
  const category = $("filter-category").value;
  if (category && category !== "All") params.set("category", category);
  if ($("filter-from").value) params.set("from", $("filter-from").value);
  if ($("filter-to").value) params.set("to", $("filter-to").value);

  expenses = await api("GET", `${API}?${params.toString()}`);
  expenses.sort((a, b) => b.date.localeCompare(a.date) || b.id - a.id);
  renderTable();
}

async function loadSummary() {
  const s = await api("GET", "/api/summary");

  $("total").textContent = money(s.total);
  $("count").textContent = s.count;

  const entries = Object.entries(s.byCategory).sort((a, b) => b[1] - a[1]);
  $("top").textContent = entries.length && entries[0][1] > 0 ? entries[0][0] : "-";

  const largest = Math.max(...entries.map((e) => e[1]), 0);
  $("breakdown").innerHTML = entries.map(([cat, amount]) => {
    const share = s.total > 0 ? (amount / s.total) * 100 : 0;
    const width = largest > 0 ? (amount / largest) * 100 : 0;
    return `
      <div class="bar-row">
        <div class="bar-label">
          <span>${esc(cat)}</span>
          <span>${money(amount)} (${share.toFixed(1)}%)</span>
        </div>
        <div class="bar"><div style="width:${width}%"></div></div>
      </div>`;
  }).join("");
}

function renderTable() {
  $("rows").innerHTML = expenses.map((e) => `
    <tr>
      <td>${esc(e.date)}</td>
      <td><span class="tag">${esc(e.category)}</span></td>
      <td>${esc(e.description || "")}</td>
      <td class="num">${money(e.amount)}</td>
      <td class="actions">
        <button class="btn small" onclick="startEdit(${e.id})">Edit</button>
        <button class="btn small danger" onclick="removeExpense(${e.id})">Delete</button>
      </td>
    </tr>`).join("");

  $("empty").hidden = expenses.length > 0;
}

async function refreshAll() {
  await Promise.all([loadExpenses(), loadSummary()]);
}

// ---------- Form: add / edit ----------
function resetForm() {
  editingId = null;
  form.reset();
  $("date").value = todayString();
  $("category").value = CATEGORIES[0];
  $("form-title").textContent = "Add Expense";
  $("submit-btn").textContent = "Add Expense";
  $("cancel-btn").hidden = true;
}

function startEdit(id) {
  const e = expenses.find((x) => x.id === id);
  if (!e) return;

  editingId = id;
  $("amount").value = e.amount;
  $("category").value = e.category;
  $("date").value = e.date;
  $("description").value = e.description || "";
  $("form-title").textContent = `Edit Expense #${id}`;
  $("submit-btn").textContent = "Save Changes";
  $("cancel-btn").hidden = false;
  window.scrollTo({ top: 0, behavior: "smooth" });
}

async function removeExpense(id) {
  if (!confirm("Delete this expense?")) return;
  try {
    await api("DELETE", `${API}/${id}`);
    if (editingId === id) resetForm();
    toast("Expense deleted");
    await refreshAll();
  } catch (err) {
    toast(err.message, true);
  }
}

form.addEventListener("submit", async (event) => {
  event.preventDefault();

  const data = {
    amount: parseFloat($("amount").value),
    category: $("category").value,
    date: $("date").value,
    description: $("description").value.trim()
  };

  if (!(data.amount > 0)) {
    toast("Amount must be greater than zero.", true);
    return;
  }

  try {
    if (editingId === null) {
      await api("POST", API, data);
      toast("Expense added");
    } else {
      await api("PUT", `${API}/${editingId}`, data);
      toast("Expense updated");
    }
    resetForm();
    await refreshAll();
  } catch (err) {
    toast(err.message, true);
  }
});

$("cancel-btn").addEventListener("click", resetForm);

// ---------- Filters ----------
["filter-category", "filter-from", "filter-to"].forEach((id) =>
  $(id).addEventListener("change", () => loadExpenses().catch((e) => toast(e.message, true)))
);

$("reset-filters").addEventListener("click", () => {
  $("filter-category").value = "All";
  $("filter-from").value = "";
  $("filter-to").value = "";
  loadExpenses().catch((e) => toast(e.message, true));
});

// ---------- Start ----------
function fillSelect(select, options, includeAll) {
  select.innerHTML =
    (includeAll ? `<option value="All">All categories</option>` : "") +
    options.map((c) => `<option>${c}</option>`).join("");
}

fillSelect($("category"), CATEGORIES, false);
fillSelect($("filter-category"), CATEGORIES, true);
resetForm();
refreshAll().catch((err) => toast("Cannot reach server: " + err.message, true));