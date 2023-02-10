// #include "stdc++.h"
#include <bits/stdc++.h>

using namespace std;

mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
double myRandom(double L, double R)
{
    uniform_real_distribution<> work(L, R);
    return work(rng);
}

void normalizeProbability(vector<double> &a)
{
    const int k = a.size();
    double s = 0.0;
    for (int i = 0; i < k; i++)
        s += a[i];
    for (int i = 0; i < k; i++)
        a[i] /= s;
}

int getRandomAction(const vector<double> &w)
{
    int k = w.size();

    double r = myRandom(0, 1);
    for (int i = 0; i < k; i++)
    {
        if (r <= w[i])
            return i;
        r -= w[i];
    }
    fprintf(stderr, "Error in Random!\n");
    return -1;
}

void playGame(const vector<vector<vector<double>>> payoff, double eps1, double eps2, int T, double h)
{
    const int k = payoff[0][0].size();
    vector<double> w1(k, 1), w2(k, 1);
    normalizeProbability(w1);
    normalizeProbability(w2);

    vector<vector<int>> cnt(k, vector<int>(k, 0));
    vector<int> m1(k, 0), m2(k, 0);
    double s1 = 0, s2 = 0;

    for (int t = 1; t <= T; t++)
    {
        int a1 = getRandomAction(w1);
        int a2 = getRandomAction(w2);
        cnt[a1][a2]++;
        m1[a1]++;
        m2[a2]++;
        s1 += payoff[0][a1][a2];
        s2 += payoff[1][a1][a2];

        // Update Weight
        for (int i = 0; i < k; i++)
            w1[i] *= pow(1 + eps1, payoff[0][i][a2] / h);
        for (int i = 0; i < k; i++)
            w2[i] *= pow(1 + eps2, payoff[1][a1][i] / h);
        normalizeProbability(w1);
        normalizeProbability(w2);
    }

    printf("Final Score: %.0lf vs %.0lf\n", s1, s2);
    printf("Distribution:\n");
    for (int i = 0; i < k; i++)
        for (int j = 0; j < k; j++)
        {
            printf("Action[%d][%d] -> %d (%.2lf \%)\n", i, j, cnt[i][j], 100.0 * cnt[i][j] / T);
        }

    printf("\nPlayer 1's Perspective:\n");
    printf("Follow Mediator: %.0lf\n", s1);
    for (int i = 0; i < k; i++)
    {
        double winnings = 0.0;
        for (int j = 0; j < k; j++)
            winnings += payoff[0][i][j] * m2[j];
        fprintf(stderr, "Only action %d: %.0lf\n", i, winnings);
    }

    printf("\nPlayer 2's Perspective:\n");
    printf("Follow Mediator: %.0lf\n", s2);
    for (int i = 0; i < k; i++)
    {
        double winnings = 0.0;
        for (int j = 0; j < k; j++)
            winnings += payoff[1][j][i] * m1[j];
        fprintf(stderr, "Only action %d: %.0lf\n", i, winnings);
    }
}

int main()
{
    // Format: payoff[p][a1][a2] = payoff for player p, if (a1, a2) is played.
    const vector<vector<vector<double>>> NO_TIE_RPS = {{{-6, -1, 1}, {1, -6, -1}, {-1, 1, -6}}, {{-6, 1, -1}, {-1, -6, 1}, {1, -1, -6}}};
    const vector<vector<vector<double>>> GO_TIE_RPS = {{{6, -1, 1}, {1, 6, -1}, {-1, 1, 6}}, {{6, 1, -1}, {-1, 6, 1}, {1, -1, 6}}};

    playGame(NO_TIE_RPS, 0.001, 0.001, 1000000, 1);
    return 0;
}

/*
void randomizerTest(vector<double> w)
{
    const int k = w.size();
    normalizeProbability(w);
    const int T = 100000;
    vector<int> cnt(k);
    for(int t = 1; t <= T; t++) cnt[getRandomAction(w)]++;
    // Report
    for(int i = 0; i < k; i++)
    {
        fprintf(stderr,"Action %d: %d / %d (%.2lf\%)\n", i, cnt[i], T, 100.0 * cnt[i] / T);
    }
}
*/
