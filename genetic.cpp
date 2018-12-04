#include "utils.h"
#include "genetic.h"

#include "algo/dinic.h"
#include "algo/edmondsKarp.h"
#include "algo/fordFulkerson.h"

/* -------- Edge --------*/
Edge::Edge() {}
Edge::Edge(Edge *e): s(e->s), t(e->t), c(e->c) {}
Edge::Edge(int _s, int _t, int _c): s(_s), t(_t), c(_c) {}

/* -------- Indiv --------*/
Indiv::Indiv() {}
Indiv::Indiv(int n) { if (n > 0) gene.resize(n); }
Indiv::~Indiv() { for (auto &e: gene) delete e; }

/* -------- Generation --------*/
Generation::Generation() {}
Generation::Generation(int _V, int _E, int _C, SolverType _S, GraphType _G, CrossoverType _CO):
	V(_V), E(_E), C(_C), S(_S), G(_G), CO(_CO) {
	for (int i = 0; i < populationSize; i++) {
		Indiv *indiv = new Indiv(E);
		for (int j = 0; j < E; j++)
			indiv->gene[j] = randomEdge();
		population.push_back(indiv);
	}
}
Generation::~Generation() { for (auto &i: population) delete i; }

pair<Indiv *, Indiv *> Generation::crossover(Indiv *indiv1, Indiv *indiv2) {
	Indiv *res1 = new Indiv(E), *res2 = new Indiv(E);

	if (CO == SPC) {
		int i, point = random_int(0, E - 1);
		for (i = 0; i <= point; i++) {
			res1->gene[i] = new Edge(indiv1->gene[i]);
			res2->gene[i] = new Edge(indiv2->gene[i]);
		}
		for (; i < E; i++) {
			res1->gene[i] = new Edge(indiv2->gene[i]);
			res2->gene[i] = new Edge(indiv1->gene[i]);
		}
	}

	if (CO == TPCS) {

	}

	return { res1, res2 };
}

Indiv *Generation::mutation(Indiv *indiv) {
	Indiv *res = new Indiv(E);

	for (int i = 0; i < E; i++) {
		if (random_real(0, E) < 1)
			res->gene[i] = randomEdge();
		else
			res->gene[i] = new Edge(indiv->gene[i]);
	}

	return res;
}

long long Generation::fitness(Indiv *a) {
	if (S == DINIC) {
		Dinic d(a, V);
		return d.match(0, V - 1);
	}

	if (S == EC) {
		EdmondsKarp d(a, V);
		return d.match(0, V - 1);
	}

	if (S == FF) {
		FordFulkerson d(a, V);
		return d.match(0, V - 1);
	}

	return 0;
}

Indiv *Generation::sizeManipulation(Indiv *indiv, int V_change, int E_change) {
	/* Logic
		Original graph size is (V, E).
		V_change and E_change can be negative.

		1. Fit vertex size into V + V_change.
			1-1. When increasing vertex size,
					randomly select |V_change| vertices from [0, V-1] and split it into two vertices.
			1-2. When splitting each vertex i,
					- newly added vertex becomes i+1 and the following vertices become i+2, i+3, ... , V.
					- all incoming edges (for original i) attach to i / all outgoing edges attach to i+1.
					- add one random edge from i to i+1.
			1-3. When decreasing vertex size,
					- randomly select |V_change| vertices from [0, V-2].
					- If we select vertex i, merge it with i+1 into one vertex.
		2. Fit edge size into E + E_change.
			2-1. When increasing edge size,
					add random edges into a random position in the gene.
			2-2. When decreasing edge size,
					delete randomly selected edges.
	*/

	int i;
	int A = max({ 0, V_change, E_change });	// # of added edges
	int D = (E + A) - (E + E_change);		// # of deleted edges

	vector<Edge *> edges_kept(E), edges_added(A);
	Indiv *res = new Indiv();

	/* ------------ 1. Fit vertex size ----------- */
	auto splitted = random_distinct_int(0, V - 1, V_change);
	auto merged = random_distinct_int(0, V - 2, -V_change);
	vector<int> state(V, 0);		// -1: merged, 0: none, 1: splitted
	vector<int> index_change(V, 0);	// index change table of "old" vertices

	for (i = 0; i < V_change; i++)
		state[splitted[i]] = 1;
	for (i = 0; i < -V_change; i++)
		state[merged[i]] = -1;

	int dist;
	for (i = dist = 0; i < V; dist += state[i++])
		index_change[i] = i + dist;

	for (i = 0; i < E; i++) {
		Edge *edge = indiv->gene[i];
		int s = index_change[edge->s];
		int t = index_change[edge->t];
		// edge->s is splitted and the edge is outgoing.
		if (state[edge->s] == 1) s++;
		edges_kept[i] = new Edge(s, t, edge->c);
	}
	/* ------------------------------------------- */

	/* ------------- 2. Fit Edge size ------------ */
	for (i = 0; i < A; i++) {
		if (i < V_change)	// edge between splitted vertices
			edges_added[i] = new Edge(splitted[i], splitted[i] + 1, random_int(1, C));
		else
			edges_added[i] = randomEdge();
	}

	auto added = random_distinct_int(0, E + A - 1, A);		// position of added edges
	auto deleted = random_distinct_int(0, E + A - 1, D);	// position of deleted edges

	int it_added = 0, it_deleted = 0;
	for (i = 0; i < E + A; i++) {
		if (i == deleted[it_deleted])
			it_deleted++;
		else if (i == added[it_added])
			res->gene.push_back(edges_added[it_added++]);
		else
			res->gene.push_back(edges_kept[i - it_deleted - it_added]);
	}
	/* ------------------------------------------- */

	return res;
}

// construct a random edge according to graph type.
Edge *Generation::randomEdge() {
	int s = random_int(0, V - 1);
	int t = random_int(0, V - 1);
	if (G == AC && s > t) swap(s, t);
	return new Edge(s, t, random_int(1, C));
}