#include "leptjson.h"
#include <vector>
#include <iostream>

using namespace std;
using namespace json;
int main() {
    string s;
    cin >> s;
    leptjson json;
    value v = json.parse(s);
    vector<vector<int>> vv(v.size(), vector<int>(v[0].size()));
    for(int i = 0; i < (int)v.size(); ++i)
        for(int j = 0; j < (int)v[0].size(); ++j)
            vv[i][j] = v[i][j].getnumber();
    for(int i = 0; i < vv.size(); ++i) {
        for(int j = 0; j < vv[0].size(); ++j)
            cout << vv[i][j] << " ";
        cout << endl;
    }
}
