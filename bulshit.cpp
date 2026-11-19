#include <bits/extc++.h>
using namespace std;
using namespace __gnu_pbds;

template <typename T>
using ordered_set = tree<T, null_type, less<T>, rb_tree_tag, tree_order_statistics_node_update>;

using ll = long long;
using ull = unsigned long long;
using ld = long double;

#if defined(_WIN32) || defined(_WIN64)
    #define gc getchar
    #define pc putchar
#else
    #define gc getchar_unlocked
    #define pc putchar_unlocked
#endif

#define debug(...) print(stderr,"[{}]:",#__VA_ARGS__),[](auto..._){(print(stderr," {}",_),...);println(stderr);}(__VA_ARGS__)

signed main(){
    cin.tie(0)->sync_with_stdio(0);


}