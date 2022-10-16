#include <windows.h>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <queue>
#include <stack>
// #define TO_TEST
#define TEST_NUM 20
#ifdef TO_TEST
#include <iomanip>
#include <fstream>
#include <algorithm>
#endif
using namespace std;

// struct of matrix
struct mat_struct
{
    int mat[3][3]; // matrix
    int steps;     // steps already take
    int pre;       // the previous step in the vector of steps
    int f;         // the heuristic function with cost i.e. steps
};

#ifdef TO_TEST
struct solve_info
{
    int node_expanded, node_to_expand;
    int steps;
    solve_info() { node_expanded = node_to_expand = steps = 0; }
};

struct game_info
{
    int order;
    solve_info single_info[4];
    solve_info &operator[](int idx) { return single_info[idx]; }
};
#endif

bool operator>(const mat_struct, const mat_struct);
#ifdef TO_TEST
bool operator<(const game_info, const game_info);
fstream &operator<<(fstream &, solve_info &);
#endif
extern int (*mat)[3];
extern const int mat_fns[3][3];
extern double scaleX, scaleY;
extern bool finished;
extern int steps;
extern int node_to_expand, node_expanded;
extern stack<mat_struct> seq;
extern wchar_t buttonLabel[5];
extern int method;
bool A_star();
void initialize();
void randomize();
int move(int, int);
#ifdef TO_TEST
void test(vector<game_info> &, int = 100);
#endif