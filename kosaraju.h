#include "encabezados.h"
#pragma once
#ifndef KOSARAJU_H
#define KOSARAJU_H


#define NIL -1
using namespace std;


// A class that represents an directed graph
class Graph
{
    int V; // No. of vertices
    list<int>* adj; // A dynamic array of adjacency lists

    //// A Recursive DFS based function used by SCC()
    //void SCCUtil(int u, int disc[], int low[],
    //    stack<int>* st, bool stackMember[]);
public:
    Graph(int V)
    {
        this->V = V;
        adj = new list<int>[V];
    }// Constructor
    void addEdge(int v, int w)
    {
        adj[v].push_back(w);
    }
    // function to add an edge to graph
   // A recursive function that finds and prints strongly connected
// components using DFS traversal
// u --> The vertex to be visited next
// disc[] --> Stores discovery times of visited vertices
// low[] -- >> earliest visited vertex (the vertex with minimum
//             discovery time) that can be reached from subtree
//             rooted with current vertex
// *st -- >> To store all the connected ancestors (could be part
//         of SCC)
// stackMember[] --> bit/index array for faster check whether
//                 a node is in stack
    void SCCUtil(int u, int disc[], int low[], stack<int>* st,  bool stackMember[], map<int, string> int_prov, map<int, list<string>>& comp, int &num_compo)
    {
        // A static variable is used for simplicity, we can avoid use
        // of static variable by passing a pointer.
        static int time = 0;
      

        // Initialize discovery time and low value
        disc[u] = low[u] = ++time;
        st->push(u);
        stackMember[u] = true;

        // Go through all vertices adjacent to this
        list<int>::iterator i;
        for (i = adj[u].begin(); i != adj[u].end(); ++i)
        {
            int v = *i; // v is current adjacent of 'u'

            // If v is not visited yet, then recur for it
            if (disc[v] == -1)
            {
                SCCUtil(v, disc, low, st, stackMember,int_prov,comp,num_compo);

                // Check if the subtree rooted with 'v' has a
                // connection to one of the ancestors of 'u'
                // Case 1 (per above discussion on Disc and Low value)
                low[u] = min(low[u], low[v]);
            }

            // Update low value of 'u' only of 'v' is still in stack
            // (i.e. it's a back edge, not cross edge).
            // Case 2 (per above discussion on Disc and Low value)
            else if (stackMember[v] == true)
                low[u] = min(low[u], disc[v]);
        }

        // head node found, pop the stack and print an SCC
        int w = 0; // To store stack extracted vertices
        if (low[u] == disc[u])
        {
            num_compo++;
            while (st->top() != u)
            {
                w = (int)st->top();
				
				//aqui se escribe otra CC.				
               // cout << int_prov[w] << " ";
                if (!comp.empty() && comp.find(num_compo) != comp.end())
                {
					comp[num_compo].push_back(int_prov[w]);
				}
				else
				{
					list<string> l;
					l.push_back(int_prov[w]);
					comp.insert(pair<int, list<string>>(num_compo, l));
                }
                
                stackMember[w] = false;
                st->pop();
            }
            w = (int)st->top();
			
			//Aquí se escribe una CC.
            //cout << int_prov[w] << "\n";
            if (!comp.empty() && comp.find(num_compo) != comp.end())
            {
                comp[num_compo].push_back(int_prov[w]);
            }
            else
            {
                list<string> l;
                l.push_back(int_prov[w]);
                comp.insert(pair<int, list<string>>(num_compo, l));
            }
			
            stackMember[w] = false;
            st->pop();
        }
    }

    // The function to do DFS traversal. It uses SCCUtil()
    void SCC(map<int, string> int_prov, map<int, list<string>> &comp)
    {
        int* disc = new int[V];
        int* low = new int[V];
        bool* stackMember = new bool[V];
        stack<int>* st = new stack<int>();
        int num_compo = 0;

        // Initialize disc and low, and stackMember arrays
        for (int i = 0; i < V; i++)
        {
            disc[i] = NIL;
            low[i] = NIL;
            stackMember[i] = false;
        }
        
        // Call the recursive helper function to find strongly
        // connected components in DFS tree with vertex 'i'
        for (int i = 0; i < V; i++)
            if (disc[i] == NIL)
                SCCUtil(i, disc, low, st, stackMember,int_prov,comp,num_compo);
    }
};





#endif