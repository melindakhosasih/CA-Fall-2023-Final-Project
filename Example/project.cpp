#include <iostream>
#include <fstream>
#include <string.h>
#include <math.h>
#include <vector>

using namespace std;

struct Cache {
    int tag;
    bool NRU;
};

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

    vector<int> index;
    for(int i = index_cnt + offset_cnt - 1; i >= offset_cnt; i--)
        index.push_back(i);
    print_idx(fout, "Indexing bits:", index);

    fin.open(argv[2], ios::in);
    run_LSB(fin, fout, address_bit - (index_cnt + offset_cnt), index_cnt, associativity);
    fin.close();
    fout.close();

    return 0;
}

