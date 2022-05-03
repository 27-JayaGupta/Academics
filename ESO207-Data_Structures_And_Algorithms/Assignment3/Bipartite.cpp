//A special property that if a graph is connected then the bipartite is unique.
#include <iostream>
using namespace std;

//node to store edges of the graph in adjacency list format
typedef struct node {
    int data;
    node* next;
} node;

//makes a new node
node* make_node(int val) {
    node* temp = new node();
    temp->data = val;
    temp->next = NULL;

    return temp;
}

//stores graph
class Graph {
  public:
    int v, e;  //v->no of vertices of graph      e->no of edges in graph 
    node** edge;

    Graph(int, int);
    void addEdge(int, int);
    void scanGraph();
};

Graph::Graph(int v_, int e_) {
    v = v_;
    e = e_;
    edge = new node*[v];
    for (int i = 0; i < v; i++) {
        edge[i] = NULL;
    }
}

void Graph::addEdge(int a, int b) {
    node* t1 = make_node(a);
    node* t2 = make_node(b);

    t1->next = edge[b];
    t2->next = edge[a];

    edge[b] = t1;
    edge[a] = t2;
}

void Graph::scanGraph() {
    int a, b;
    int e_ = e;
    cout << "Enter edges one by one in newline ";
    while (e_--) {
        cin >> a >> b;
        addEdge(a, b);
    }
}

bool visited[100001] = {false};   //will store whether a vertex is visited or not
int section[100001] = {-1};       // will store in which section (V1 or V2) do a given vertex lies

// i-> vertex which is being explored
// sec-> section to which the vertex i belongs
//returns false if a graph cannnot be bipartite
//returns true if a graph until vertex i is bipartitie
bool dfs(Graph g, int i, int sec) {
    visited[i] = true;
    section[i] = sec;

    int alt_sec = (sec == 1 ? 2 : 1);    // alt_sec stores the section in which the adjacent vertex to i which are not                                            // visited will be stored

    node* curr = g.edge[i];    //we will now explore all the adjacent vertices to i
    while (curr != NULL) {
        if (!visited[curr->data]) {      //if the vertex is not visited ,visit that vertex
            bool ans = dfs(g, curr->data, alt_sec);   
            if (ans == false)       // if ans==false means graph cannot be bipartite hence return false from here 
                return false;       //(no need of exploring forward)
        } else {                 // if the node is already visited then it should be in alt-sec for it to be bipartite
            if (section[curr->data] == sec) { // if adjacent vertex in same section as i (cant be bipartite)
                return false;
            }
        }
        curr = curr->next;
    }

    return true;
}

// Vertices from 0 to v-1
//will return whether a graph is bipartite or not
bool Bipartite(Graph g) {
    int v_ = g.v;
    int e_ = g.e;

    bool ans = dfs(g, 0, 1);     // Vertex 0 is by default kept in section 1

    return ans;
}

int main() {
    int v, e;
    cout << "Enter number of vertices in the graph: ";
    cin >> v;
    cout << "Enter number of edges in the graph: ";
    cin >> e;

    Graph g(v, e);
    g.scanGraph();

    bool result = Bipartite(g);
    if (result) {    // if ans ==true print sections of all the vertices
        cout << "YES" << endl;
        for (int i = 0; i < v; ++i)
            cout << section[i] << endl;
    } else
        cout << "NO" << endl;
}
