(() => {
  "use strict";
  const input = document.querySelector("[data-site-search]");
  const results = document.querySelector("[data-search-results]");
  if (!input || !results) return;
  let entries = [];
  let ready = false;
  const close = () => { results.classList.remove("open"); results.innerHTML = ""; };
  const esc = (s) => String(s ?? "").replace(/[&<>"']/g, c => ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
  const normalize = s => String(s || "").toLowerCase().normalize("NFKD").replace(/[\u0300-\u036f]/g, "");
  fetch("/search-index.json", {headers: {"Accept":"application/json"}})
    .then(r => { if (!r.ok) throw new Error("Search index unavailable"); return r.json(); })
    .then(data => { entries = Array.isArray(data) ? data : []; ready = true; })
    .catch(() => { ready = false; });
  function search() {
    const query = normalize(input.value.trim());
    if (!query) { close(); return; }
    if (!ready) {
      results.innerHTML = '<div class="search-empty">Search is loading. You can also browse the project links below.</div>';
      results.classList.add("open"); return;
    }
    const terms = query.split(/\s+/).filter(Boolean);
    const found = entries.map(item => {
      const haystack = normalize([item.title,item.description,(item.keywords || []).join(" "),item.category].join(" "));
      const score = terms.reduce((sum, term) => sum + (haystack.includes(term) ? (normalize(item.title).includes(term) ? 4 : 1) : -10), 0);
      return {item,score};
    }).filter(x => x.score >= 0 && terms.every(term => normalize([x.item.title,x.item.description,(x.item.keywords || []).join(" "),x.item.category].join(" ")).includes(term)))
      .sort((a,b) => b.score-a.score).slice(0,8);
    results.innerHTML = found.length ? found.map(({item}) =>
      '<a class="search-result" href="' + esc(item.url) + '"><strong>' + esc(item.title) + '</strong><small>' + esc(item.description) + '</small></a>'
    ).join("") : '<div class="search-empty">No matching pages. Try “Android”, “bytecode”, “CMLL”, “install” or “GitHub”.</div>';
    results.classList.add("open");
  }
  input.addEventListener("input", search);
  input.addEventListener("focus", search);
  input.addEventListener("keydown", e => { if (e.key === "Escape") close(); if (e.key === "Enter") { const first = results.querySelector("a"); if (first) location.href = first.href; } });
  document.addEventListener("click", e => { if (!e.target.closest(".search-wrap")) close(); });
  document.addEventListener("keydown", e => { if (e.key === "/" && !/input|textarea/i.test(document.activeElement.tagName)) { e.preventDefault(); input.focus(); } });
})();
