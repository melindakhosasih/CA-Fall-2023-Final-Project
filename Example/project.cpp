#include <iostream>
#include <fstream>
#include <string.h>
#include <math.h>
#include <vector>
#include <set>

using namespace std;

struct Cache {
    int tag;
    bool NRU;
};

int min(int a, int b) {
    return (a > b) ? b : a;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

void print_out(ofstream& fout, string str, int val) {
    fout << str << val << endl;
}

void print_idx(ofstream& fout, string str, vector<int> index) {
    fout << str;
    for (int i = 0; i < index.size(); i++)
        fout << " " << index[i];
    fout << endl << endl;
}

void run_LSB(ifstream& fin, ofstream& fout, int tag_cnt, int index_cnt, int n_way) {
    // cout << "tag bit " << tag_cnt << endl;
    // cout << "index bit " << index_cnt << endl;
    // cout << "sets " << (1 << index_cnt) << endl;

    string str, title;
    vector<string> addresses;
    vector<Cache> caches[n_way];
    
    for (int i = 0; i < n_way; i++) {
        caches[i].resize(1 << index_cnt);
        // Initialize caches
        // cout << "caches size " << caches[i].size() << endl;
        for (int j = 0; j < caches[i].size(); j++) {
            caches[i][j].tag = -1;
            caches[i][j].NRU = true;
        }
    }

    // Debug
    // for (int i = 0; i < n_way; i++) {
    //     for (int j = 0; j < caches[i].size(); j++) {
    //         cout << caches[i][j].tag << " ";
    //         cout << caches[i][j].NRU << endl;
    //     }
    // }

    // Scan reference
    getline(fin, title);
    while (getline(fin, str)) {
        if (str == ".end") break;
        addresses.push_back(str);
    }
    
    // Execute
    int total_miss = 0;
    fout << title << endl;
    for (int i = 0; i < addresses.size(); i++) {
        bool miss = true;
        fout << addresses[i];
        string tag = addresses[i].substr(0, tag_cnt);
        string index = addresses[i].substr(tag_cnt, index_cnt);
        int tag_int = stoi(tag, nullptr, 2);
        int index_int = stoi(index, nullptr, 2);
        // cout << "tag " << tag << " " << tag_int << endl;
        // cout << "index " << index << " " << index_int << endl;

        // Check for hit
        for(int i = 0; i < n_way; i++) {
            if(caches[i][index_int].tag == tag_int) {
                fout << " hit" << endl;
                caches[i][index_int].NRU = false;
                miss = false;
            }
        }

        while (miss) {
            bool hit = false;
            // Find first 1
            for(int i = 0; i < n_way; i++) {
                if(caches[i][index_int].NRU) {
                    fout << " miss" << endl;
                    total_miss++;
                    caches[i][index_int].tag = tag_int;
                    caches[i][index_int].NRU = false;
                    hit = true;
                    break;
                }
            }

            // All 0 -> Reset all to 1
            if (!hit) {
                for(int i = 0; i < n_way; i++) {
                    caches[i][index_int].NRU = true;
                }
            } else {
                break;
            }
        }
    }
    fout << ".end" << endl << endl;
    print_out(fout, "Total cache miss count: ", total_miss);
}

void run_zero_cost(ifstream& fin, ofstream& fout, int index_cnt, int offset_cnt, int len, int n_way) {
    // cout << "tag bit " << tag_cnt << endl;
    // cout << "index bit " << index_cnt << endl;
    // cout << "sets " << (1 << index_cnt) << endl;

    string str, title;
    vector<string> addresses;
    vector<Cache> caches[n_way];
    
    for (int i = 0; i < n_way; i++) {
        caches[i].resize(1 << index_cnt);
        // Initialize caches
        // cout << "caches size " << caches[i].size() << endl;
        for (int j = 0; j < caches[i].size(); j++) {
            caches[i][j].tag = -1;
            caches[i][j].NRU = true;
        }
    }

    // Debug
    // for (int i = 0; i < n_way; i++) {
    //     for (int j = 0; j < caches[i].size(); j++) {
    //         cout << caches[i][j].tag << " ";
    //         cout << caches[i][j].NRU << endl;
    //     }
    // }

    // Scan reference
    getline(fin, title);
    while (getline(fin, str)) {
        if (str == ".end") break;
        addresses.push_back(str);
    }

    // Calculate Quality Measurement (Q)
    // Count zero and one
    int zero[len] = {0}, one[len] = {0};
    double Q[len];
    for (int i = 0; i < addresses.size(); i++) {
        for (int j = 0; j < len; j++) {
            if (addresses[i][j] == '1') one[j]++;
            else zero[j]++;
        }
    }

    // Debug
    // for (int i = 0; i < len; i++) {
    //     cout << "one: " << one[i] << " zero: " << zero[i] << endl;
    // }

    // Compute Q
    for (int i = 0; i < len; i++)
        Q[i] = double(min(zero[i], one[i])) / double(max(zero[i], one[i]));

    // Debug
    // cout << "Quality" << endl;
    // for (int i = 0; i < len; i++)
    //     cout << Q[i] << " ";
    // cout << endl;


    // Calculate Correlation (C)
    double C[len][len];
    for (int i = 0; i < len; i++) {
        for (int j = 0 ; j < len; j++) {
            if (i == j) C[i][j] = 0;
            else if (i > j) C[i][j] = C[j][i];
            else {
                int same = 0, diff = 0;
                for(int k = 0; k < addresses.size(); k++) {
                    if(addresses[k][i] == addresses[k][j]) same++;
                    else diff++;
                }
                C[i][j] = double(min(same, diff)) / double(max(same, diff));
            }
        }
    }

    // Debug
    // cout << "correlation" << endl;
    // for (int i = 0; i < len; i++) {
    //     for (int j = 0; j < len; j++) {
    //         cout << C[i][j] << " ";
    //     }
    //     cout << endl;
    // }

    // Calculate Near-optimal Index Ordering
    set<int> indices;

    for (int i = 0; i < index_cnt; i++) {
        double max = -1;
        int best_q;

        // Debug
        // cout << "Updated Quality" << endl;
        // for (int j = 0; j < len; j++) {
        //     cout << Q[j] << " ";
        // }
        // cout << endl;

        for(int j = 0; j < len; j++) {
            auto it = indices.find(j);
            if (it == indices.end()) {  // No duplicate
                if (max <= Q[j]) {
                    max = Q[j];
                    best_q = j;
                }
            }
        }

        // Update Quality
        for(int j = 0; j < len; j++)
            Q[j] *= C[j][best_q];

        indices.insert(best_q);
    }

    // Debug
    // for (auto it = indices.begin(); it != indices.end(); it++)
    //     cout << *it << " ";
    // cout << endl;

    // Calculate indices
    vector<int> tags_sorted, indices_sorted;
    auto it = indices.begin();
    for (int i = len + offset_cnt - 1; i >= offset_cnt; i--) {
        // if (it != indices.end())
        // cout << "check " << (len + offset_cnt - 1 - (*it)) << endl;
        if(i == (len + offset_cnt - 1 - (*it)) && it != indices.end()) {
            indices_sorted.push_back(i);
            it++;
        } else tags_sorted.push_back(i);
    }
    print_idx(fout, "Indexing bits:", indices_sorted);

    // Debug
    // cout << "tags:";
    // for (int i = 0; i < tags_sorted.size(); i++) {
    //     cout << " " << tags_sorted[i];
    // }
    // cout << endl << "indices:";
    // for (int i = 0; i < indices_sorted.size(); i++) {
    //     cout << " " << indices_sorted[i];
    // }
    // cout << endl;
    
    // Execute
    int total_miss = 0;
    fout << title << endl;
    for (int i = 0; i < addresses.size(); i++) {
        bool miss = true;
        fout << addresses[i];

        string tag, index;
        for (int j = 0; j < tags_sorted.size(); j++) {
            tag += addresses[i][(len + offset_cnt - 1) - tags_sorted[j]];
        }
        for (int j = 0; j < indices_sorted.size(); j++) {
            index += addresses[i][(len + offset_cnt - 1) - indices_sorted[j]];
        }

        // cout << "final tag " << tag << endl;
        // cout << "final index " << index << endl;

        int tag_int = stoi(tag, nullptr, 2);
        int index_int = stoi(index, nullptr, 2);
        // cout << "tag " << tag << " " << tag_int << endl;
        // cout << "index " << index << " " << index_int << endl;

        // Check for hit
        for(int i = 0; i < n_way; i++) {
            if(caches[i][index_int].tag == tag_int) {
                fout << " hit" << endl;
                caches[i][index_int].NRU = false;
                miss = false;
            }
        }

        while (miss) {
            bool hit = false;
            // Find first 1
            for(int i = 0; i < n_way; i++) {
                if(caches[i][index_int].NRU) {
                    fout << " miss" << endl;
                    total_miss++;
                    caches[i][index_int].tag = tag_int;
                    caches[i][index_int].NRU = false;
                    hit = true;
                    break;
                }
            }

            // All 0 -> Reset all to 1
            if (!hit) {
                for(int i = 0; i < n_way; i++) {
                    caches[i][index_int].NRU = true;
                }
            } else {
                break;
            }
        }
    }
    fout << ".end" << endl << endl;
    print_out(fout, "Total cache miss count: ", total_miss);
}

int main(int argc, char *argv[]){
    ifstream fin;
    ofstream fout;

    fout.open(argv[3], ios::out);

    // Scan and print cache file inputs
    string str;
    int address_bit, block_size, cache_sets, associativity;
    fin.open(argv[1], ios::in);

    // Address bit
    fin >> str >> address_bit;
    print_out(fout, "Address bits: ", address_bit);
    // Block size
    fin >> str >> block_size;
    print_out(fout, "Block size: ", block_size);
    // Cache sets
    fin >> str >> cache_sets;
    print_out(fout, "Cache sets: ", cache_sets);
    // Associativity
    fin >> str >> associativity;
    print_out(fout, "Associativity: ", associativity);
    fout << endl;
    fin.close();

    // Count offset bit
    int offset_cnt = log2(block_size);
    print_out(fout, "Offset bit count: ", offset_cnt);
    // Count index bit
    int index_cnt = log2(cache_sets);
    print_out(fout, "Indexing bit count: ", index_cnt);

    // vector<int> index;
    // for(int i = index_cnt + offset_cnt - 1; i >= offset_cnt; i--)
    //     index.push_back(i);
    // print_idx(fout, "Indexing bits:", index);

    fin.open(argv[2], ios::in);
    // run_LSB(fin, fout, address_bit - (index_cnt + offset_cnt), index_cnt, associativity);
    run_zero_cost(fin, fout, index_cnt, offset_cnt, address_bit - offset_cnt, associativity);
    fin.close();
    fout.close();

    return 0;
}

