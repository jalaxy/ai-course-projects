#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
using namespace std;
#define HEIGHT 16
#define WIDTH 30
#define BLANK '_' // related to input
#define NONE ' '  // related to clauses
#define POS '1'
#define NEG '0'
#define DIF '*'
#define forall(i) for (int i = 0; i < HEIGHT * WIDTH; i++)

struct clause
{
    unsigned char seq[HEIGHT * WIDTH]; // the sequence of every blank
    // different value:
    //   NONE -> no literals
    //   POS  -> positive literal -> x
    //   NEG  -> negative literal -> ~x
    //   DIF  -> used when mergeing, representing different literals
    clause()
    {
        forall(i) seq[i] = NONE;
    }
    clause(int pos, unsigned char x) // set blank in pos to x
    {
        forall(i) seq[i] = NONE;
        seq[pos] = x;
    }
    unsigned char &operator[](int i) { return seq[i]; }
    clause operator+(clause &b) // resolve the inverse literals
    {
        clause c;
        forall(i)
        {
            unsigned char la = seq[i], lb = b.seq[i];
            if (la == NONE)
                c.seq[i] = lb;
            else if (lb == NONE)
                c.seq[i] = la;
            else if (la != lb)
                c.seq[i] = NONE;
            else
                c.seq[i] = la;
        }
        return c;
    }
    clause operator*(clause &b) // merge two clauses
    {
        clause c;
        forall(i)
        {
            unsigned char la = seq[i], lb = b.seq[i];
            if (la == DIF || lb == DIF)
                c.seq[i] = DIF;
            else if (la == NONE)
                c.seq[i] = lb;
            else if (lb == NONE)
                c.seq[i] = la;
            else if (la != lb)
                c.seq[i] = DIF;
            else
                c.seq[i] = la;
        }
        return c;
    }
    bool operator&&(clause &b) // wheather resolvable
    {
        int num = 0;
        forall(i)
        {
            unsigned char la = seq[i], lb = b.seq[i];
            if (la != NONE && lb != NONE && la != lb)
                num++;
        }
        return num == 1;
    }
    bool operator<(clause &b) // compare the number of literals
    {
        int n = 0, nb = 0;
        forall(i)
        {
            if (this->seq[i] != NONE)
                n++;
            if (b.seq[i] != NONE)
                nb++;
        }
        return n < nb;
    }
    bool operator==(const clause &b)
    {
        for (int i = 0; i < HEIGHT * WIDTH; i++)
            if (this->seq[i] != b.seq[i])
                return false;
        return true;
    }
    bool operator!=(clause &b) { return !(*this == b); }
    unsigned char operator<<(clause &b) // assign b to *this (without changing clauses)
    {
        bool none = false;
        for (int i = 0; i < HEIGHT * WIDTH; i++)
            if (seq[i] != NONE)
                if (b.seq[i] != NONE)
                {
                    if (seq[i] == b.seq[i])
                        return POS;
                }
                else
                    none = true;
        return none ? NONE : NEG;
    }
    unsigned char operator<<=(clause &b) // assign b to *this (remove reverse literals)
    {
        bool none = false;
        for (int i = 0; i < HEIGHT * WIDTH; i++)
            if (seq[i] != NONE)
                if (b.seq[i] != NONE)
                    if (seq[i] == b.seq[i])
                        return POS;
                    else
                        seq[i] = NONE;
                else
                    none = true;
        return none ? NONE : NEG;
    }
    int num()
    {
        int cnt = 0;
        for (int i = 0; i < HEIGHT * WIDTH; i++)
            if (seq[i] != NONE)
                cnt++;
        return cnt;
    }
    int num(int &last)
    {
        int cnt = 0;
        for (int i = 0; i < HEIGHT * WIDTH; i++)
            if (seq[i] != NONE)
            {
                cnt++;
                last = i;
            }
        return cnt;
    }
    int at(int n) // return the index of (n + 1)th symbol
    {
        for (int i = 0; i < HEIGHT * WIDTH; i++)
            if (seq[i] != NONE)
            {
                if (n == 0)
                    return i;
                n--;
            }
        return -1;
    }
};

unsigned char mat[HEIGHT][WIDTH]; // input mine area
vector<int> mdl[8][9];            // model to generate CNF
// mdl[i - 1][j] means truth table where there are j mines in i blanks
const clause null;

bool isnumber(char ch) { return ch >= '0' && ch <= '8'; }

/* generate CNF model
 * param: none
 * retval: none (global variable mdl)
 */
void generate_model()
{
    for (int i = 1; i <= 8; i++)
        for (int j = 0; j <= i; j++)
            for (int k = 0; k < 1 << i; k++)
            {
                int num = 0, x = k;
                for (int l = 0; l < i; l++)
                    num += (x >> l) & 1;
                if (num != j)
                    mdl[i - 1][j].push_back(k);
            }
}

/* generate clauses from a number square
 * param: the position of a number square
 * retval: a vector of clauses
 */
vector<clause> generate_from_num(int i, int j)
{
    vector<clause> vc = {};
    if (i < 0 || j >= WIDTH || !isnumber(mat[i][j]))
        return vc;
    int n = 0;
    int ii[8], jj[8];
    n += (int)(i > 0 && j > 0 && mat[ii[n] = i - 1][jj[n] = j - 1] == BLANK);
    n += (int)(i > 0 && mat[ii[n] = i - 1][jj[n] = j] == BLANK);
    n += (int)(i > 0 && j < WIDTH - 1 && mat[ii[n] = i - 1][jj[n] = j + 1] == BLANK);
    n += (int)(j > 0 && mat[ii[n] = i][jj[n] = j - 1] == BLANK);
    n += (int)(j < WIDTH - 1 && mat[ii[n] = i][jj[n] = j + 1] == BLANK);
    n += (int)(i < HEIGHT - 1 && j > 0 && mat[ii[n] = i + 1][jj[n] = j - 1] == BLANK);
    n += (int)(i < HEIGHT - 1 && mat[ii[n] = i + 1][jj[n] = j] == BLANK);
    n += (int)(i < HEIGHT - 1 && j < WIDTH - 1 && mat[ii[n] = i + 1][jj[n] = j + 1] == BLANK);
    if (n == 0)
        return vc;
    vector<int> lst = mdl[n - 1][mat[i][j] - '0'];
    for (int k = 0; k < lst.size(); k++)
    {
        int x = lst[k];
        clause c;
        for (int idx = 0; idx < n; idx++)
            c[ii[idx] * WIDTH + jj[idx]] = (x >> idx) & 1 ? NEG : POS;
        vc.push_back(c);
    }
    return vc;
}

/* generate clauses related to a blank square in radius
 * param: the position of a waiting-for-determine square
 * retval: a vector of clauses
 */
vector<clause> generate_for_blank(int i, int j, int radius)
{
    vector<clause> vc = {};
    if (mat[i][j] != BLANK)
        return vc;
    vector<int> di, dj;
    for (int ii = -radius; ii <= radius; ii++)
        for (int jj = -radius; jj <= radius; jj++)
            if (ii != 0 || jj != 0)
            {
                di.push_back(ii);
                dj.push_back(jj);
            }
    for (int idx = 0; idx < di.size(); idx++)
    {
        int idi = i + di[idx], jdj = j + dj[idx];
        if (idi >= 0 && idi <= HEIGHT - 1 &&
            jdj >= 0 && jdj <= WIDTH - 1 && isnumber(mat[idi][jdj]))
        {
            vector<clause> tmp = generate_from_num(idi, jdj);
            vc.insert(vc.end(), tmp.begin(), tmp.end());
        }
    }
    for (int ii = 0; ii < vc.size(); ii++) // reduce the repeated clauses
    {
        bool flag = false;
        for (int jj = 0; jj < ii; jj++)
            if (vc[ii] == vc[jj])
                flag = true;
        if (flag)
        {
            vc.erase(vc.begin() + ii);
            ii--;
        }
    }
    return vc;
}

/* resolve vc
 * param: clauses
 * retval: whether the CNF of these clauses is satisfiable
 */
bool resolve(vector<clause> vc, clause &model)
{
    int size = vc.size();
    for (int i = 0; i < size; i++)
        if (vc[i] == null)
            return false;
    for (int ii = 0; ii < 2; ii++)
    {
        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
                if (vc[i] && vc[j])
                {
                    clause new_clause = vc[i] + vc[j];
                    if (new_clause == null)
                        return false;
                    bool repeated = false;
                    for (int k = 0; k < vc.size(); k++)
                        if (new_clause == vc[k])
                            repeated = true;
                    if (!repeated)
                        vc.push_back(new_clause);
                }
        if (size == vc.size())
            break;
        size = vc.size();
    }
    return true;
}

/* DPLL algorithm
 * param: clauses and reference of their assignments
 * retval: whether CNF of the clauses is satisfiable (model returned by reference)
 * note: model need to be initialized by null clause
 */
bool DPLL(vector<clause> vc, clause &model)
{
    bool pos = true; // simply test CNF value in model
    for (int i = 0; i < vc.size(); i++)
    {
        unsigned char x = vc[i] <<= model;
        pos &= x == POS;
        if (x == NEG)
            return false; // if any clause is false
        else if (x == POS)
        {
            vc.erase(vc.begin() + i);
            i--;
        }
    }
    if (pos)
        return true; // if every clause is true
    clause tot;
    for (int i = 0; i < vc.size(); i++)
        tot = tot * vc[i];
    bool pure = false; // find pure symbols
    for (int i = 0; i < HEIGHT * WIDTH; i++)
        if (tot[i] == POS || tot[i] == NEG)
        {
            pure = true;
            model[i] = tot[i];
            for (int j = 0; j < vc.size(); j++)
                if (vc[j][i] == tot[i])
                {
                    vc.erase(vc.begin() + j);
                    j--;
                }
        }
    if (pure)
        return DPLL(vc, model);
    for (int i = 0, last; i < vc.size(); i++)
        if (vc[i].num(last) == 1)
        {
            model[last] = vc[i][last]; // find a unit clause
            vc.erase(vc.begin() + i);
            return DPLL(vc, model);
        }
    int first = 0;
    clause model_tmp = model;
    for (int i = 0; i < vc.size(); i++)
        for (int j = 0; j < HEIGHT * WIDTH; j++)
            if (vc[i][j] != NONE && model[j] == NONE)
            {
                model[j] = POS; // finally test both situation of the first symbol
                if (DPLL(vc, model))
                    return true;
                model = model_tmp;
                model[j] = NEG;
                if (DPLL(vc, model))
                    return true;
                return false;
            }
    return true;
}

unsigned int rand_uint(int n = 32)
{
    srand(time(NULL));
    int ans = 0;
    for (int i = 0; i < n; i++)
        ans |= (int)(rand() % 2) << i;
    return ans;
}

/* WalkSAT algorithm
 * param: the CNF to find a solution and the reference of assignment
 * retval: whether found (model returned by reference)
 */
bool WalkSAT(vector<clause> vc, clause &model)
{
    model = null;
    int max_flips = 1000;
    double p = 0.5;
    for (int i = 0; i < vc.size(); i++)
        for (int j = 0; j < HEIGHT * WIDTH; j++)
            if (vc[i][j] != NONE && model[j] == NONE)
                model[j] = (bool)(rand_uint(1)) ? POS : NEG; // randomly decide assignments
    for (int times = 0; times < max_flips; times++)
    {
        int cnt = 0;
        for (int i = 0; i < vc.size(); i++)
            if (vc[i] << model == NEG)
                cnt++; // find all the false clause in the model
        if (cnt == 0)
            return true; // all the clauses are satisfied
        clause c;
        cnt = rand_uint() % cnt; // randomly choose one clause and assign to c
        for (int i = 0; i < vc.size() && cnt >= 0; i++)
            if (vc[i] << model == NEG)
            {
                if (cnt == 0)
                    c = vc[i];
                cnt--;
            }
        if (rand_uint(8) / 256.0 < p)
        {
            unsigned char &x = model[c.at(rand_uint() % c.num())];
            x = x == POS ? NEG : POS; // randomly choose a symbol to flip at probability p
        }
        else
        {
            int num_max = 0; // choose symbol with the most satisfied clauses to flip otherwise
            clause model_new;
            for (int i = 0; i < HEIGHT * WIDTH; i++)
                if (c[i] != NONE)
                {
                    int num = 0; // the number of satisfied clauses
                    clause model_tmp = model;
                    model_tmp[i] = model_tmp[i] == POS ? NEG : POS;
                    for (int i = 0; i < vc.size(); i++)
                        if (vc[i] << model_tmp == POS)
                            num++;
                    if (num > num_max)
                    {
                        model_new = model_tmp;
                        num_max = num;
                    }
                }
            model = model_new;
        }
    }
    return false; // cannot find a satisfiable in max_flips
}

/* run an algorithm to find blocks with mines and without mines
 * param: reference of vectors of integer represents 1-D position of mines and blanks, 
 *        also the function pointer of solve method and search range
 * retval: none (mines and blanks returned by reference)
 */
void run(vector<int> &mines, vector<int> &blanks,
         bool method(vector<clause>, clause &),
         int lower_bound = 0, int upper_bound = 30)
{
    for (int radius = lower_bound; radius <= upper_bound; radius++)
    {
        vector<clause> vc_mine_0;
        for (int i = 0; i < mines.size(); i++)
            vc_mine_0.push_back(clause(mines[i], POS)); // previous infomation
        vector<int>().swap(mines);                      // clear mines
        for (int i = 0; i < HEIGHT; i++)
            for (int j = 0; j < WIDTH; j++)
                if (mat[i][j] == BLANK)
                {
                    vector<clause> vc_mine = generate_for_blank(i, j, radius),
                                   vc_blank = vc_mine;
                    if (vc_mine.empty())
                        continue;
                    vc_mine.insert(vc_mine.begin(), vc_mine_0.begin(), vc_mine_0.end());
                    vc_mine.insert(vc_mine.begin(), clause(i * WIDTH + j, NEG));
                    vc_blank.insert(vc_blank.begin(), clause(i * WIDTH + j, POS));
                    clause model;
                    if (!method(vc_mine, model = null))
                        mines.push_back(i * WIDTH + j);
                    else if (!method(vc_blank, model = null))
                        blanks.push_back(i * WIDTH + j);
                }
        if (!blanks.empty())
            break; // until find a blank
    }
}

int main()
{
    /***********************************************************************************
      The communication with minesweeper program is implemented by files.
      Input file is the map, which uses only "_" and numbers(1-8) to represent squares
          to determine and numbers of mines around.
      Output file is the position of blanks and mines found, following the format below:
      m n
          (m is the number of blanks, which is the squares without mines)
          (n is the number of mines)
      x_blanks_1    y_blanks_1
      x_blanks_2    y_blanks_2
      ......
      x_blanks_m    y_blanks_m
      x_mines_1     y_mines_1
      x_mines_2     y_mines_2
      ......
      x_mines_n     y_mines_n
    ***********************************************************************************/
    system("cls");
    cout << "Enter \"q\" or press \"Ctrl+C\" to quit\n"
         << "Press Enter to continue...";
    double total_time = .0;
    // loop to continuously run solution
    while (true)
    {
        string s;
        getline(cin, s);
        if (s == "q" || cin.fail())
            break;
        clock_t t_s = clock();
        fstream in, out;
        in.open("board.txt", ios::in);
        for (int i = 0; i < HEIGHT; i++)
            for (int j = 0; j < WIDTH; j++)
                in >> mat[i][j];
        in.close();

        generate_model();
        vector<int> mines, blanks;
        run(mines, blanks, DPLL);

        out.open("mines.txt", ios::out);
        out << blanks.size() << " " << mines.size() << endl
            << endl;
        for (int i = 0; i < blanks.size(); i++)
            out << blanks[i] / WIDTH << " " << blanks[i] % WIDTH << endl;
        out << endl;
        for (int i = 0; i < mines.size(); i++)
            out << mines[i] / WIDTH << " " << mines[i] % WIDTH << endl;
        out.close();
        double single_time = (float)(clock() - t_s) / CLOCKS_PER_SEC;
        total_time += single_time;
        cout << "Elapsed time: " << single_time << endl
             << "Total time: " << total_time << endl
             << "Mines: " << mines.size() << endl
             << "Blanks: " << blanks.size() << endl
             << "Press Enter to continue...";
    }
    return 0;
}
