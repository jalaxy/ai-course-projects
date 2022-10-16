#include "game_new.h"

const int m = 1000007;
int n = 3;
matState mat;
int *mat_fns;
bool finished;
int steps;
int method = 1;
int node_to_expand, node_expanded;
stack<matState> seq;
vector<matState> mat_tmp, visited[m];

bool operator>(const matState a, const matState b) { return a.f > b.f; }
int hmt(matState mat)
{
    int hmt = 0;
    int *i_fns = new int[n * n + 1], *j_fns = new int[n * n + 1];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
        {
            i_fns[mat_fns[i * n + j]] = i;
            j_fns[mat_fns[i * n + j]] = j;
        }
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            hmt += abs(i - i_fns[mat[i * n + j]]) +
                   abs(j - j_fns[mat[i * n + j]]);
    delete[] i_fns;
    delete[] j_fns;
    return hmt;
}

int h(matState mat)
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

int order(matState mat)
{
    int ans = 0;
    int *prod = new int[n * n];
    prod[0] = 1;
    for (int i = 1; i < n * n; i++)
        prod[i] = prod[i - 1] % m * i % m;
    for (int i = 0; i < n * n; i++)
    {
        int cnt = 0;
        for (int j = i + 1; j < n * n; j++)
            if (mat[i] > mat[j])
                ans = (ans + prod[n * n - 1 - i]) % m;
    }
    delete[] prod;
    return ans;
}

bool A_star()
{
    return 1;
    A_star_pq pq;
    vector<matState> mat_tmp;
    pq.push(mat);
    while (!pq.empty())
    {
        matState cur = pq.top();
        mat_tmp.push_back(cur);
        pq.pop();
        if (cur == mat_fns)
        {
            steps = cur.steps;
            break;
        }
        int i_blank, j_blank;
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (cur[i * n + j] == n * n)
                {
                    i_blank = i;
                    j_blank = j;
                }
        matState tmp1 = cur, tmp2 = cur, tmp3 = cur, tmp4 = cur;
        if (i_blank > 0)
        {
            tmp1.steps++;
            tmp1[i_blank * n + j_blank] = tmp1[(i_blank - 1) * n + j_blank];
            tmp1[(i_blank - 1) * n + j_blank] = n * n;
            tmp1.f = h(tmp1) + tmp1.steps;
            tmp1.pre = mat_tmp.size() - 1;
            int ord = order(tmp1);
            bool flag = 1;
            for (int i = 0; i < visited[ord].size(); i++)
                if (tmp1 == visited[ord][i])
                {
                    flag = 0;
                    break;
                }
            if (flag)
            {
                visited[ord].push_back(tmp1);
                pq.push(tmp1);
            }
        }
        if (i_blank < n - 1)
        {
            tmp2.steps++;
            tmp2[i_blank * n + j_blank] = tmp2[(i_blank + 1) * n + j_blank];
            tmp2[(i_blank + 1) * n + j_blank] = n * n;
            tmp2.f = h(tmp2) + tmp2.steps;
            tmp2.pre = mat_tmp.size() - 1;
            int ord = order(tmp2);
            bool flag = 1;
            for (int i = 0; i < visited[ord].size(); i++)
                if (tmp2 == visited[ord][i])
                {
                    flag = 0;
                    break;
                }
            if (flag)
            {
                visited[ord].push_back(tmp2);
                pq.push(tmp2);
            }
        }
        if (j_blank > 0)
        {
            tmp3.steps++;
            tmp3[i_blank * n + j_blank] = tmp3[i_blank * n + j_blank - 1];
            tmp3[i_blank * n + j_blank - 1] = n * n;
            tmp3.f = h(tmp3) + tmp3.steps;
            tmp3.pre = mat_tmp.size() - 1;
            int ord = order(tmp3);
            bool flag = 1;
            for (int i = 0; i < visited[ord].size(); i++)
                if (tmp3 == visited[ord][i])
                {
                    flag = 0;
                    break;
                }
            if (flag)
            {
                visited[ord].push_back(tmp3);
                pq.push(tmp3);
            }
        }
        if (j_blank < n - 1)
        {
            tmp4.steps++;
            tmp4[i_blank * n + j_blank] = tmp4[i_blank * n + j_blank + 1];
            tmp4[i_blank * n + j_blank + 1] = n * n;
            tmp4.f = h(tmp4) + tmp4.steps;
            tmp4.pre = mat_tmp.size() - 1;
            int ord = order(tmp4);
            bool flag = 1;
            for (int i = 0; i < visited[ord].size(); i++)
                if (tmp4 == visited[ord][i])
                {
                    flag = 0;
                    break;
                }
            if (flag)
            {
                visited[ord].push_back(tmp4);
                pq.push(tmp4);
            }
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

void initialize()
{
    if (mat_fns != NULL)
        delete[] mat_fns;
    mat_fns = new int[n * n];
    for (int i = 0; i < n * n; i++)
        mat_fns[i] = i + 1;
    finished = 0;
    finished = 0;
    steps = 0;
    node_to_expand = node_expanded = 0;
    wcscpy_s(buttonLabel, L"Calc");
    while (!seq.empty())
        seq.pop();
    for (int i = 0; i < m; i++)
        visited[i].clear();
}

void randomize()
{
    bool *v = new bool[n * n];
    for (int i = 0; i < n * n; i++)
        v[i] = 0;
    int i = 0;
    mat.Create_new_mat();
    while (i < n * n)
    {
        mat.mat_1d[i] = rand() % (n * n) + 1;
        if (v[mat.mat_1d[i] - 1] == 0)
            v[mat.mat_1d[i++] - 1] = 1;
    }
    delete[] v;
    int inv = 0, inv_fns = 0, i_n2;
    for (int i = 0; i < n * n; i++)
    {
        if (mat[i] == n * n)
            i_n2 = i / n;
        for (int j = i + 1; j < n * n; j++)
        {
            if (mat.mat_1d[i] < n * n && mat.mat_1d[j] < n * n && mat.mat_1d[i] > mat.mat_1d[j])
                inv++;
            if (mat_fns[i] < n * n && mat_fns[j] < n * n && mat_fns[i] > mat_fns[j])
                inv_fns++;
        }
    }
    if (n % 2 == 0)
        inv += i_n2;
    if ((inv + inv_fns) % 2 != 0)
        for (int i = 1; i < n * n; i++)
            if (mat.mat_1d[i - 1] < n * n && mat.mat_1d[i] < n * n && mat.mat_1d[i - 1] > mat.mat_1d[i])
            {
                int tmp = mat.mat_1d[i - 1];
                mat.mat_1d[i - 1] = mat.mat_1d[i];
                mat.mat_1d[i] = tmp;
                break;
            }
}

void release()
{
    if (mat_fns != NULL)
        delete[] mat_fns;
}

int move(int i_mov, int j_mov)
{
    if (mat[i_mov * n + j_mov] == n * n)
        return 0;
    int i, j;
    i = i_mov;
    j = j_mov;
    while (i-- > 0 && mat[i * n + j] != n * n)
        ;
    if (i >= 0)
    {
        for (int ii = i; ii < i_mov; ii++)
        {
            mat[ii * n + j] = mat[(ii + 1) * n + j];
            steps++;
        }
        mat[i_mov * n + j] = n * n;
        return !memcmp(mat.mat_1d, mat_fns, n * n * sizeof(int));
    }
    i = i_mov;
    j = j_mov;
    while (i++ < n - 1 && mat[i * n + j] != n * n)
        ;
    if (i <= n - 1)
    {
        for (int ii = i; ii > i_mov; ii--)
        {
            mat[ii * n + j] = mat[(ii - 1) * n + j];
            steps++;
        }
        mat[i_mov * n + j] = n * n;
        return !memcmp(mat.mat_1d, mat_fns, n * n * sizeof(int));
    }
    i = i_mov;
    j = j_mov;
    while (j-- > 0 && mat[i * n + j] != n * n)
        ;
    if (j >= 0)
    {
        for (int jj = j; jj < j_mov; jj++)
        {
            mat[i * n + jj] = mat[i * n + jj + 1];
            steps++;
        }
        mat[i * n + j_mov] = n * n;
        return !memcmp(mat.mat_1d, mat_fns, n * n * sizeof(int));
    }
    i = i_mov;
    j = j_mov;
    while (j++ < n - 1 && mat[i * n + j] != n * n)
        ;
    if (j <= n - 1)
    {
        for (int jj = j; jj > j_mov; jj--)
        {
            mat[i * n + jj] = mat[i * n + jj - 1];
            steps++;
        }
        mat[i * n + j_mov] = n * n;
        return !memcmp(mat.mat_1d, mat_fns, n * n * sizeof(int));
    }
    return 0;
}