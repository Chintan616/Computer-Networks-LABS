#include <bits/stdc++.h>
using namespace std;

struct Connection {
    int cost;
    int next;
    int parent;
    int edges;

    Connection(){
        this->cost = INT_MAX;
        this->next = -1;
        this->parent = -1;
        this->edges = 0;
    }

    Connection(int cost, int next, int parent) {
        this->cost = cost;
        this->next = next;
        this->parent = parent;
        this->edges = 0;
    }
};
void print_routing_table(vector<vector<Connection>> &dist, int i, int n){
    cout << "Dest\t" << "Cost\t" << "Next\t" << "Parent\t" << "Flag\t" << "Path\t" << endl;
    for(int j=0; j<n; j++){
        cout << j <<"\t" 
             << dist[i][j].cost << "\t" 
             << dist[i][j].next  << "\t" 
             << dist[i][j].parent << "\t";

        cout << "U";
        if(dist[i][j].next != j & dist[i][j].next != -1)
            cout << "G";
        cout << "\t";

        vector<int> path;
        int k = j;
        while(dist[i][k].parent != -1){
            path.push_back(k);
            k = dist[i][k].parent;
        }
        path.push_back(k);

        for(int temp=(int)path.size()-1; temp>=0; temp--)
            cout << path[temp] << " -> ";
        cout << endl;
    }
    cout<< endl;
}
void shortest_path(const vector<vector<pair<int, int>>> &adj, vector<Connection> &dist, int i, int n) {
    dist[i].cost = 0;

    priority_queue<int, vector<int>, greater<>> pq;
    vector<bool> vis(n, false);
    pq.push(i);

    while(!pq.empty()) {
        int u = pq.top();
        pq.pop();

        if(vis[u] == true)
            continue;
        vis[u] = true;

        for(auto [nei, cost] : adj[u]){
            if((dist[nei].cost > dist[u].cost + cost) || (dist[nei].cost >= dist[u].cost + cost && dist[nei].edges > dist[u].edges + 1)){
                dist[nei].cost = dist[u].cost + cost;   // cost
                dist[nei].parent = u;                   // parent
                dist[nei].edges = dist[u].edges + 1;    // edges
                if(u != i && dist[u].next != i)         // next
                    dist[nei].next = dist[u].next;
                else
                    dist[nei].next = nei;
                pq.push({nei});
            }
        }
    }
}

int main() {
    cout << "Number of Nodes in the Network: ";
    int n = 10;
    // cin >> n;

    // vector<vector<pair<int, int>>> adj(n, vector<pair<int, int>>());
    vector<vector<pair<int, int>>> adj = {
        {{6,4}, {1,1}, {3,1}},
        {{0,1}, {3,1}, {2,3}, {6,1}, {8,4}, {4,5}},
        {{1,3}, {5,4}, {9,3}},
        {{0,1}, {1,1}, {4,2}},
        {{3,2}, {5,3}, {1,5}},
        {{2,4}, {4,3}},
        {{0,4}, {1,1}, {7,1}},
        {{6,1}, {8,2}},
        {{7,2}, {9,8}, {1,4}},
        {{8,8}, {2,3}}
    }; // neighbour, cost

    cout << endl << "Press -1 when all neighbours of a router has been provided" << endl << endl;

    //  input
    // input(adj, n);

    vector<vector<Connection>> dist(n, vector<Connection>(n));
    for(int i=0; i<n; i++){
        shortest_path(adj, dist[i], i, n);
    }
    
    while(1) {
        cout << "Give the router number to see routing table: ";
        int i;
        cin >> i;

        print_routing_table(dist, i, n);
    }

    return 0;
}