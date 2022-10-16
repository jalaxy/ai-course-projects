#include "game.h"

int (*mat)[4];
const int mat_fns[4][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
double scaleX = 1, scaleY = 1;
bool finished;
int steps;
int method = 1;
int node_to_expand, node_expanded;
stack<mat_struct> seq;
vector<mat_struct> visited[mod];

bool operator>(const mat_struct a, const mat_struct b) { return a.f > b.f; }
#ifdef TO_TEST
fstream &operator<<(fstream &fs, solve_info &info)
{
    fs << setw(8) << info.steps
       << setw(8) << info.node_expanded
       << setw(8) << info.node_to_expand;
    return fs;
}
bool operator<(const game_info a, const game_info b)
{
    return a.single_info[0].steps < b.single_info[0].steps;
}
#endif
typedef priority_queue<mat_struct, vector<mat_struct>, greater<mat_struct>>
    A_star_pq;

int hmt(int (*mat)[4])
{
    int hmt = 0;
    int i_fns[17], j_fns[17];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
        {
            i_fns[mat_fns[i][j]] = i;
            j_fns[mat_fns[i][j]] = j;
        }
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (mat[i][j] < 16) // optimality
                hmt += abs(i - i_fns[mat[i][j]]) +
                       abs(j - j_fns[mat[i][j]]);
    return hmt;
}

int h(int (*mat)[4])
{
    int d = hmt(mat);
    switch (method)
    {
    case 0:
        return 0;
    case 1:
        return d;
    case 2:
        return 2 * d;
    case 3:
        return 10 * d;
    }
    return d;
}

int order(int (*mat)[4])
{
    int ans = 0;
    int prod[16] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320};
    prod[0] = 1;
    for (int i = 1; i < 16; i++)
        prod[i] = prod[i - 1] % mod * i % mod;
    int *mat_1d = (int *)mat;
    for (int i = 0; i < 16; i++)
    {
        int cnt = 0;
        for (int j = i + 1; j < 16; j++)
            if (mat_1d[i] > mat_1d[j])
                ans = (ans + prod[15 - i]) % mod;
    }
    return ans;
}

bool A_star()
{
    A_star_pq pq;
    vector<mat_struct> mat_tmp;
    mat_struct ini;
    ini.f = ini.steps = 0;
    ini.pre = 0;
    memcpy(ini.mat, mat, sizeof(int) * 16);
    pq.push(ini);
    while (!pq.empty())
    {
        mat_struct cur = pq.top();
        mat_tmp.push_back(cur);
        pq.pop();
        if (!memcmp(cur.mat, mat_fns, sizeof(int) * 16))
        {
            steps = cur.steps;
            break;
        }
        int i_blank, j_blank;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                if (cur.mat[i][j] == 16)
                {
                    i_blank = i;
                    j_blank = j;
                }
        mat_struct tmp = cur;
        if (i_blank > 0)
        {
            tmp.mat[i_blank][j_blank] = tmp.mat[i_blank - 1][j_blank];
            tmp.mat[i_blank - 1][j_blank] = 16;
            tmp.f = h(tmp.mat) + ++tmp.steps;
            tmp.pre = mat_tmp.size() - 1;
            int ord = order(tmp.mat);
            bool flag = 0;
            for (int i = 0; i < visited[ord].size(); i++)
                if (memcmp(tmp.mat, visited[ord][i].mat, 16 * sizeof(int)) == 0)
                {
                    if (tmp.f < visited[ord][i].f)
                    {
                        visited[ord][i].f = tmp.f;
                        pq.push(tmp);
                    }
                    flag = 1;
                    break;
                }
            if (!flag)
            {
                pq.push(tmp);
                visited[ord].push_back(tmp);
            }
            tmp = cur;
        }
        if (i_blank < 3)
        {
            tmp.mat[i_blank][j_blank] = tmp.mat[i_blank + 1][j_blank];
            tmp.mat[i_blank + 1][j_blank] = 16;
            tmp.f = h(tmp.mat) + ++tmp.steps;
            tmp.pre = mat_tmp.size() - 1;
            int ord = order(tmp.mat);
            bool flag = 0;
            for (int i = 0; i < visited[ord].size(); i++)
                if (memcmp(tmp.mat, visited[ord][i].mat, 16 * sizeof(int)) == 0)
                {
                    if (tmp.f < visited[ord][i].f)
                    {
                        visited[ord][i].f = tmp.f;
                        pq.push(tmp);
                    }
                    flag = 1;
                    break;
                }
            if (!flag)
            {
                pq.push(tmp);
                visited[ord].push_back(tmp);
            }
            tmp = cur;
        }
        if (j_blank > 0)
        {
            tmp.mat[i_blank][j_blank] = tmp.mat[i_blank][j_blank - 1];
            tmp.mat[i_blank][j_blank - 1] = 16;
            tmp.f = h(tmp.mat) + ++tmp.steps;
            tmp.pre = mat_tmp.size() - 1;
            int ord = order(tmp.mat);
            bool flag = 0;
            for (int i = 0; i < visited[ord].size(); i++)
                if (memcmp(tmp.mat, visited[ord][i].mat, 16 * sizeof(int)) == 0)
                {
                    if (tmp.f < visited[ord][i].f)
                    {
                        visited[ord][i].f = tmp.f;
                        pq.push(tmp);
                    }
                    flag = 1;
                    break;
                }
            if (!flag)
            {
                pq.push(tmp);
                visited[ord].push_back(tmp);
            }
            tmp = cur;
        }
        if (j_blank < 3)
        {
            tmp.mat[i_blank][j_blank] = tmp.mat[i_blank][j_blank + 1];
            tmp.mat[i_blank][j_blank + 1] = 16;
            tmp.f = h(tmp.mat) + ++tmp.steps;
            tmp.pre = mat_tmp.size() - 1;
            int ord = order(tmp.mat);
            bool flag = 0;
            for (int i = 0; i < visited[ord].size(); i++)
                if (memcmp(tmp.mat, visited[ord][i].mat, 16 * sizeof(int)) == 0)
                {
                    if (tmp.f < visited[ord][i].f)
                    {
                        visited[ord][i].f = tmp.f;
                        pq.push(tmp);
                    }
                    flag = 1;
                    break;
                }
            if (!flag)
            {
                pq.push(tmp);
                visited[ord].push_back(tmp);
            }
            tmp = cur;
        }
    }
    node_expanded = mat_tmp.size();
    node_to_expand = pq.size();
    int p = mat_tmp.size() - 1;
    while (p > 0)
    {
        seq.push(mat_tmp[p]);
        p = mat_tmp[p].pre;
    }
    return mat_tmp.size() > 1;
}

int move(int i_mov, int j_mov)
{
    if (mat[i_mov][j_mov] == 16)
        return 0;
    int i, j;
    i = i_mov;
    j = j_mov;
    while (i-- > 0 && mat[i][j] != 16)
        ;
    if (i >= 0)
    {
        for (int ii = i; ii < i_mov; ii++)
        {
            mat[ii][j] = mat[ii + 1][j];
            steps++;
        }
        mat[i_mov][j] = 16;
        return !memcmp(mat, mat_fns, 16 * sizeof(int));
    }
    i = i_mov;
    j = j_mov;
    while (i++ < 3 && mat[i][j] != 16)
        ;
    if (i <= 3)
    {
        for (int ii = i; ii > i_mov; ii--)
        {
            mat[ii][j] = mat[ii - 1][j];
            steps++;
        }
        mat[i_mov][j] = 16;
        return !memcmp(mat, mat_fns, 16 * sizeof(int));
    }
    i = i_mov;
    j = j_mov;
    while (j-- > 0 && mat[i][j] != 16)
        ;
    if (j >= 0)
    {
        for (int jj = j; jj < j_mov; jj++)
        {
            mat[i][jj] = mat[i][jj + 1];
            steps++;
        }
        mat[i][j_mov] = 16;
        return !memcmp(mat, mat_fns, 16 * sizeof(int));
    }
    i = i_mov;
    j = j_mov;
    while (j++ < 3 && mat[i][j] != 16)
        ;
    if (j <= 3)
    {
        for (int jj = j; jj > j_mov; jj--)
        {
            mat[i][jj] = mat[i][jj - 1];
            steps++;
        }
        mat[i][j_mov] = 16;
        return !memcmp(mat, mat_fns, 16 * sizeof(int));
    }
    return 0;
}

void randomize()
{
    int *mat_1d = NULL, *mat_fns_1d = (int *)mat_fns;
    while (mat_1d == NULL)
        mat_1d = new int[16];
    mat = reinterpret_cast<int(*)[4]>(mat_1d);
    int v[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int i = 0;
    while (i < 16)
    {
        mat_1d[i] = rand() % 16 + 1;
        if (v[mat_1d[i] - 1] == 0)
            v[mat_1d[i++] - 1] = 1;
    }
    int inv = 0, inv_fns = 0, i_16;
    // int mat__[3][3] = {{1, 7, 6}, {8, 5, 4}, {2, 3, 9}};
    // int mat__[3][3] = {{8, 6, 7}, {9, 5, 1}, {4, 3, 2}};
    // int mat__[3][3] = {{7, 9, 4}, {1, 6, 8}, {5, 3, 2}};
    // int mat__[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    // int mat__[3][3] = {{9, 8, 7}, {6, 5, 4}, {3, 2, 1}};
    // memcpy(mat, mat__, 36);
    for (int i = 0; i < 16; i++)
    {
        for (int j = i + 1; j < 16; j++)
        {
            if (mat_1d[i] < 16 && mat_1d[j] < 16 && mat_1d[i] > mat_1d[j])
                inv++;
            if (mat_fns_1d[i] < 16 && mat_fns_1d[j] < 16 && mat_fns_1d[i] > mat_fns_1d[j])
                inv_fns++;
        }
        if (mat_1d[i] == 16)
            i_16 = i / 4;
    }
    inv += (i_16 + 1) % 2;
    if ((inv + inv_fns) % 2 != 0)
        for (int i = 1; i < 16; i++)
            if (mat_1d[i - 1] < 16 && mat_1d[i] < 16 && mat_1d[i - 1] > mat_1d[i])
            {
                int tmp = mat_1d[i - 1];
                mat_1d[i - 1] = mat_1d[i];
                mat_1d[i] = tmp;
                break;
            }
}

void initialize()
{
    finished = 0;
    steps = 0;
    node_to_expand = node_expanded = 0;
    wcscpy(buttonLabel, L"Calc");
    memset(visited, 0, sizeof(visited));
    for (int i = 0; i < mod; i++)
        visited[i].clear();
    while (!seq.empty())
        seq.pop();
}

#ifdef TO_TEST
void test(vector<game_info> &info, int num)
{
    bool flag = num == -1;
    if (num == -1)
        num = 362880;
    int *mat_1d = reinterpret_cast<int *>(mat);
    if (flag)
        for (int i = 0; i < 16; i++)
            mat_1d[i] = i + 1;
    else
        randomize();
    int valid = 1;
    while (num--)
    {
        game_info new_info;
        new_info.order = order(mat);
        if (valid)
        {
            for (int i = 0; i < 4; i++)
            {
                initialize();
                method = i;
                A_star();
                new_info[i].steps = steps;
                new_info[i].node_expanded = node_expanded;
                new_info[i].node_to_expand = node_to_expand;
            }
            info.push_back(new_info);
        }
        if (flag)
        {
            int i = 8;
            while (i > 0 && mat_1d[i] < mat_1d[i - 1])
                i--;
            if (i == 0)
                break;
            int j = i--;
            while (j < 16 && mat_1d[j] > mat_1d[i])
                j++;
            j--;
            int tmp = mat_1d[i];
            mat_1d[i] = mat_1d[j];
            mat_1d[j] = tmp;
            for (int k = i + 1; k <= (i + 16) / 2; k++)
            {
                int tmp = mat_1d[k];
                mat_1d[k] = mat_1d[i + 16 - k];
                mat_1d[i + 16 - k] = tmp;
            }
            int inv = 0;
            for (int i = 0; i < 16; i++)
                for (int j = i + 1; j < 16; j++)
                    if (mat_1d[i] < 16 && mat_1d[j] < 16 && mat_1d[i] > mat_1d[j])
                        inv++;
            valid = !(inv % 2);
        }
        else
            randomize();
    }
}
#endif