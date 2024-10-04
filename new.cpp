#include <vector>
#include <unordered_map>

using namespace std;

class Solution {
public:
    int findSpecialInteger(vector<int>& arr) {
        unordered_map<int, int> counts;
        int times = static_cast<int>(arr.size() * 0.25);
        for (int n : arr) {
            counts[n]++;
        }

        for (pair<int, int> p : counts) {
            if (p.second == times) {
                return p.first;
            }
        }

        return 0;
    }
};