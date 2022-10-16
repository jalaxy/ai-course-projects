#include <windows.h>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <queue>
#include <stack>
using namespace std;

extern int n;

struct matState
{
    int *mat_1d = NULL;
    int f, steps;
    int pre;
    void Create_new_mat()
    {
        f = 0;
        steps = 0;
        pre = 0;
        if (mat_1d != NULL)
            delete[] mat_1d;
        mat_1d = new int[n * n];
    }
    matState() { Create_new_mat(); }
    matState(const matState &b) { *this = b; }
    ~matState() { delete[] mat_1d; }
    matState &operator=(const matState &b)
    {
        f = b.f;
        steps = b.steps;
        pre = b.pre;
        mat_1d = new int[n * n];
        memcpy(mat_1d, b.mat_1d, n * n * sizeof(int));
        return *this;
    }
    int &operator[](int i) { return mat_1d[i]; }
    bool operator==(int *mat_fns)
    {
        return !memcmp(mat_1d, mat_fns, sizeof(int) * n * n);
    }
    bool operator==(matState b)
    {
        return *this == b.mat_1d;
    }
    bool operator!=(int *mat_fns) { return !(*this == mat_fns); }
};

typedef priority_queue<matState, vector<matState>, greater<matState>>
    A_star_pq;

extern matState mat;
extern int *mat_fns;
extern bool finished;
extern int steps;
extern int node_to_expand, node_expanded;
extern int method;
extern stack<matState> seq;

extern double scaleX, scaleY;
extern RECT rectButton1, rectButton2;
extern RECT rectSelect1, rectSelect2, rectSelect3, rectSelect4;
extern wchar_t buttonLabel[5];

bool operator>(const matState, const matState);
void initialize();
void release();
void randomize();
int move(int, int);
int h(matState *mat);
bool A_star();