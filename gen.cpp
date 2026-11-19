// Graph stress test generator template for competitive programming.
//
// Usage: ./gen key=value key=value ...      (a bare `flag` means flag=1; 2e5 style numbers are OK)
//
//   seed=1            RNG seed
//   type=graph        tree | graph | dag | bip
//   n=10  nmin=n      n is drawn uniformly from [nmin, n]
//   m=15  mmin=m      m is drawn uniformly from [mmin, m], then clamped to what is feasible
//   T=0               if > 0: multitest, prints T first, then T cases
//   connected=1       graph/dag/bip: start from a random spanning tree (weakly connected)
//   shape=random      tree skeleton: random | path | star | caterpillar | broom | kary | deep | prufer
//   k=2               kary: branching factor, deep: parent is among the previous k vertices
//   directed=0        (dag is always directed, edges go from lower to higher id BEFORE relabeling)
//   multi=0           allow parallel edges
//   loops=0           allow self-loops (graph type only)
//   weighted=0        wl=1 wr=1e9    edge weights drawn from [wl, wr]
//   base=1            vertex indexing base (0 or 1)
//   pm=1              print m after n (default: 0 for type=tree, 1 otherwise)
//   relabel=1         randomly permute vertex labels (hides structure like topological order)
//   shuf=1            shuffle edge order
//
// Examples:
//   ./gen seed=5 type=tree n=2e5 shape=path
//   ./gen seed=7 n=1000 m=5000 weighted wl=-100 wr=100 directed
//   ./gen seed=3 type=dag n=8 m=12 connected=0
//   ./gen seed=9 type=bip n=10 m=20
//   ./gen seed=1 T=100 nmin=1 n=8 mmin=0 m=12 connected=0     # many tiny cases
//
// Edit `emit()` to match the input format of the problem (queries, extra header, etc).

#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef pair<int, int> pii;

mt19937_64 rng;
ll rnd(ll l, ll r) { return uniform_int_distribution<ll>(l, r)(rng); }

map<string, string> A;
string gs(const string &k, const string &d) {
    auto it = A.find(k);
    return it == A.end() ? d : it->second;
}
ll gi(const string &k, ll d) {
    auto it = A.find(k);
    return it == A.end() ? d : llround(stold(it->second));
}

struct Cfg {
    string type, shape;
    bool directed, multi, loops, connected, weighted, relabel, shuf, pm;
    int k, base;
    ll wl, wr;
} C;

// ---------------------------------------------------------------- trees
// Returns n-1 edges on labels 0..n-1 (for all shapes except prufer, parent < child).
vector<pii> genTree(int n, const string &shape, int k) {
    vector<pii> e;
    if (n <= 1) return e;
    if (shape == "prufer") {  // uniform over all labeled trees
        if (n == 2) return {{0, 1}};
        vector<int> p(n - 2), deg(n, 1);
        for (auto &x : p) x = rnd(0, n - 1), deg[x]++;
        set<int> leaves;
        for (int i = 0; i < n; i++) if (deg[i] == 1) leaves.insert(i);
        for (int x : p) {
            int l = *leaves.begin();
            leaves.erase(leaves.begin());
            e.push_back({l, x});
            if (--deg[x] == 1) leaves.insert(x);
        }
        e.push_back({*leaves.begin(), *leaves.rbegin()});
        return e;
    }
    for (int i = 1; i < n; i++) {
        int p;
        if (shape == "path") p = i - 1;
        else if (shape == "star") p = 0;
        else if (shape == "kary") p = (i - 1) / max(1, k);
        else if (shape == "deep") p = rnd(max(0, i - k), i - 1);
        else if (shape == "caterpillar") p = i < n / 2 ? i - 1 : rnd(0, n / 2 - 1);
        else if (shape == "broom") p = i < n / 2 ? i - 1 : n / 2 - 1;
        else p = rnd(0, i - 1);  // random
        e.push_back({p, i});
    }
    return e;
}

// Random spanning tree that only uses edges between the two sides [0, n/2) and [n/2, n).
vector<pii> bipTree(int n) {
    vector<pii> e;
    if (n < 2) return e;
    int L = n / 2;
    vector<int> order = {0, L};
    vector<int> rest;
    for (int i = 0; i < n; i++) if (i != 0 && i != L) rest.push_back(i);
    shuffle(rest.begin(), rest.end(), rng);
    order.insert(order.end(), rest.begin(), rest.end());
    vector<int> side[2];
    side[0].push_back(0);
    side[1].push_back(L);
    e.push_back({0, L});
    for (size_t i = 2; i < order.size(); i++) {
        int v = order[i], s = v < L ? 0 : 1;
        int p = side[1 - s][rnd(0, (int)side[1 - s].size() - 1)];
        e.push_back({p, v});
        side[s].push_back(v);
    }
    return e;
}

// ---------------------------------------------------------------- general graphs
// number of distinct simple edges available (ignores `multi`)
ll capacity(int n) {
    ll N = n;
    if (C.type == "tree") return max(0, n - 1);
    if (C.type == "bip") return n < 2 ? 0 : (C.directed ? 2 : 1) * (N / 2) * (N - N / 2);
    if (C.type == "dag") return N * (N - 1) / 2;
    ll c = C.directed ? N * (N - 1) : N * (N - 1) / 2;
    return c + (C.loops ? N : 0);
}

bool okPair(int n, int u, int v) {
    if (C.type == "dag") return u < v;
    if (C.type == "bip") return (u < n / 2) != (v < n / 2);
    return C.loops || u != v;
}

pii sample(int n) {
    while (true) {
        int u = rnd(0, n - 1), v = rnd(0, n - 1);
        if (C.type == "dag" && u > v) swap(u, v);
        if (okPair(n, u, v)) return {u, v};
    }
}

ll key(int n, int u, int v) {
    if (!C.directed && u > v) swap(u, v);
    return (ll)u * n + v;
}

vector<pii> build(int n, int m) {
    vector<pii> E;
    unordered_set<ll> used;
    auto add = [&](int u, int v) {
        E.push_back({u, v});
        used.insert(key(n, u, v));
    };
    if (C.type == "tree" || C.connected) {
        auto t = C.type == "bip" ? bipTree(n) : genTree(n, C.shape, C.k);
        for (auto [u, v] : t) {
            if (C.type == "dag") { if (u > v) swap(u, v); }
            else if (C.directed && rnd(0, 1)) swap(u, v);
            add(u, v);
        }
    }
    int need = m - (int)E.size();
    if (need > 0) {
        ll cap = capacity(n);
        bool dense = !C.multi && (ll)need * 3 > cap - (ll)E.size();
        if (dense) {  // enumerate all remaining pairs (O(n^2), fine for n <= ~5000)
            vector<pii> all;
            for (int u = 0; u < n; u++)
                for (int v = 0; v < n; v++) {
                    if (!C.directed && u > v) continue;
                    if (!okPair(n, u, v) || used.count(key(n, u, v))) continue;
                    all.push_back({u, v});
                }
            shuffle(all.begin(), all.end(), rng);
            for (int i = 0; i < need; i++) add(all[i].first, all[i].second);
        } else {
            while (need > 0) {
                auto [u, v] = sample(n);
                if (!C.multi && used.count(key(n, u, v))) continue;
                add(u, v);
                need--;
            }
        }
    }
    return E;
}

// ---------------------------------------------------------------- output
// >>> Edit this to match the problem's input format <<<
void emit(int n, vector<pii> &E) {
    vector<int> p(n);
    iota(p.begin(), p.end(), 0);
    if (C.relabel) shuffle(p.begin(), p.end(), rng);
    for (auto &[u, v] : E) {
        u = p[u], v = p[v];
        if (!C.directed && rnd(0, 1)) swap(u, v);
    }
    if (C.shuf) shuffle(E.begin(), E.end(), rng);
    cout << n;
    if (C.pm) cout << ' ' << E.size();
    cout << '\n';
    for (auto [u, v] : E) {
        cout << u + C.base << ' ' << v + C.base;
        if (C.weighted) cout << ' ' << rnd(C.wl, C.wr);
        cout << '\n';
    }
}

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        string s = argv[i];
        auto p = s.find('=');
        if (p == string::npos) A[s] = "1";
        else A[s.substr(0, p)] = s.substr(p + 1);
    }
    rng.seed(gi("seed", 1));

    C.type = gs("type", "graph");
    C.shape = gs("shape", "random");
    C.k = gi("k", 2);
    C.directed = gi("directed", 0) || C.type == "dag";
    C.multi = gi("multi", 0);
    C.loops = gi("loops", 0) && C.type == "graph";
    C.connected = gi("connected", 1);
    C.weighted = gi("weighted", 0);
    C.relabel = gi("relabel", 1);
    C.shuf = gi("shuf", 1);
    C.pm = gi("pm", C.type != "tree");
    C.base = gi("base", 1);
    C.wl = gi("wl", 1);
    C.wr = gi("wr", 1e9);

    ll n = gi("n", 10), m = gi("m", n - 1);
    ll nmin = gi("nmin", n), mmin = gi("mmin", m);
    int T = gi("T", 0);

    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (T > 0) cout << T << '\n';
    for (int tc = 0; tc < max(T, 1); tc++) {
        int N = rnd(min(nmin, n), n);
        ll cap = capacity(N);
        ll hi = cap == 0 ? 0 : (C.multi && C.type != "tree" ? (ll)4e18 : cap);
        ll lo = (C.type == "tree" || C.connected) ? max(0, N - 1) : 0;
        lo = min(lo, hi);
        ll M = rnd(min(mmin, m), m);
        M = min(max(M, lo), hi);
        auto E = build(N, (int)M);
        emit(N, E);
    }
}
