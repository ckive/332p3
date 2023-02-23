#include<bits/stdc++.h>
using namespace std;

mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
double myRandom(double L, double R)
{
    uniform_real_distribution<> work(L,R);
    return work(rng);
}

double uniform()
{
    return myRandom(0, 1);
}

double mystery1()
{
    // F(t) = sqrt(t) for t in [0, 1]
    // Optimal Reserve: 1 / sqrt(3)
    return sqrt(myRandom(0, 1));
}


void normalizeProbability(vector<double> &a)
{
    const int k = a.size();
    double s = 0.0;
    for(int i = 0; i < k; i++) s += a[i];
    for(int i = 0; i < k; i++) a[i] /= s;
}

int getRandomAction(const vector<double> &w)
{
    int k = w.size();

    double r = myRandom(0, 1);
    for(int i = 0; i < k; i++)
    {
        if(r <= w[i]) return i;
        r -= w[i];
    }
    fprintf(stderr,"Error in Random!\n");
    return -1;
}

vector<double> getAuctionRevenue(int k, vector<double> b, const vector<double> &rsv) // O(n log n) due to sorting
{
    // k-item (k + 1)-th price auction
    // b: bids
    // rsv: possible reserve (in increasing order)
    // return revenue[i] = revenue if reserve is set to r[i]
    const int n = b.size();
    sort(b.begin(), b.end()); reverse(b.begin(), b.end());
    int overCount = 0;

    vector<double> revenue(rsv.size());
    for(int i = rsv.size() - 1; i >= 0; i--)
    {
        while(overCount < k && b[overCount] >= rsv[i]) overCount++;
        double winnerPay = max(b[overCount], rsv[i]);
        revenue[i] = winnerPay * overCount;
    }
    return revenue;
}


void auctionEW(int k, int bidders, int T, const double eps, const vector<double> &reserve, function<double()> F)
{
    // k-item (k + 1)-th price auction
    // "bidders" players in each of T auction
    // eps: learning rate for exponential weight
    // reserve: possible values of reserve
    // F: unknown distribution of bids

    // Weight for Exponential Weight
    vector<double> w(reserve.size(), 1);
    normalizeProbability(w);

    const double h = k; // Max Payoff per round. Now, it is assumed that $max(F()) = 1.

    double totalRevenue = 0.0;
    for(int t = 1; t <= T; t++)
    {

        vector<double> b(bidders);
        for(int i = 0; i < bidders; i++) b[i] = F();

        const int chosenAction = getRandomAction(w);
        vector<double> payoff = getAuctionRevenue(k, b, reserve);
        totalRevenue += payoff[chosenAction];

        // Update Weight
        for(int i = 0; i < w.size(); i++) w[i] *= pow(1.0 + eps, payoff[i] / h);
        normalizeProbability(w);
    }

    printf("Average Revenue: %.5lf\n", totalRevenue / T);
    printf("Final Weight:\n");
    for(int i = 0; i < w.size(); i++)
    {
        if(w[i] >= 0.001) printf("p[%.2lf] = %.3lf\n", reserve[i], w[i]);
    }

}

void tradeEW(int T, const double eps, const vector<double> &thresholds, function<double()> F, function<double()> G)
{
    // Weight for Exponential Weight
    vector<double> w(thresholds.size(), 1);
    normalizeProbability(w);

    const double h = 1; // Max Payoff per round

    double totalRevenue = 0.0;
    for(int t = 1; t <= T; t++)
    {
        const int chosenAction = getRandomAction(w);
        const double chosenCut = thresholds[chosenAction];
        // fprintf(stderr,"Chosen Cut: %.4lf\n", chosenCut);

        double v = F();
        double c = G();
        double delta = v - c;

        vector<double> payoff(thresholds.size());
        for(int i = 0; i < thresholds.size(); i++)
        {
            if(delta > thresholds[i]) payoff[i] = 2 * delta - 1;
        }
        double thisRoundRevenue = payoff[chosenAction];
        totalRevenue += thisRoundRevenue;

        // Update Weight
        for(int i = 0; i < w.size(); i++) w[i] *= pow(1.0 + eps, payoff[i] / h);
        normalizeProbability(w);
    }

    printf("Average Revenue: %.5lf\n", totalRevenue / T);
    for(int i = 0; i < w.size(); i++)
    {
        if(w[i] >= 0.001) printf("p[%.2lf] = %.3lf\n", thresholds[i], w[i]);
    }

}

// Use this to approximate theoretical optimal
double getExpectedRevenue(int k, int n, double reserve, function<double()> F, int T = 1000)
{
    double total = 0;
    vector<double> r(1, reserve);
    for(int t = 1; t <= T; t++)
    {
        vector<double> b(n);
        for(int i = 0; i < n; i++) b[i] = F();

        double thisRound = getAuctionRevenue(k, b, r)[0];
        total += thisRound;
    }
    return total / T;
}

class EstimatorAgentBruteForce
{
private:
    double blockSize;
    vector<double> blockCount;
    vector<double> S;

    double getVirtualValue(double v)
    {
        // fprintf(stderr,"Go %.4lf\n", v);
        const int n = S.size();
        sort(S.begin(), S.end());
        int idx = 0; while(S[idx + 1] < v) idx++;
        //fprintf(stderr,"Above: %d / %d\n", idx + 1, n);

        double d = S[idx + 1] - S[idx];

        double F = ((double) idx + 1.0 + (v - S[idx]) / d) / (double) n;
        // fprintf(stderr,"F: %.4lf\n", F);
        // double f = 1.0 / ((double) n * d);
        double f = blockCount[(int) (v / blockSize)] / (blockSize * n);
        //fprintf(stderr,"f: %.4lf\n", f);

        double ret = v - (1.0 - F) / f;
        //fprintf(stderr,"phi(%.4lf) = %.4lf\n", v, ret);
        return ret;
    }
public:
    double estimateCriticalValue(double eps = 0.001)
    {
        double L = 0, R = 1;
        while(R - L > eps)
        {
            double M = L + 0.5 * (R - L);
            if(getVirtualValue(M) < 0) L = M;
            else R = M;
        }
        return L + 0.5 * (R - L);
    }

    void updateValue(double v)
    {
        S.push_back(v);
        blockCount[(int) (v / blockSize)] += 1.0;
    }

    EstimatorAgentBruteForce()
    {
        int k = 30;
        blockCount = vector<double>(k + 2, 0);
        blockSize = 1.0 / (double) k;
        for(int i = 0; i <= k; i++) updateValue(blockSize * (double) i);
    }
};

void auctionEmpirical(int k, int bidders, int T, function<double()> F)
{
    // k-item (k + 1)-th price auction
    // "bidders" players in each of T auction
    // F: unknown distribution of bids

    EstimatorAgentBruteForce bot;

    double totalRevenue = 0.0;
    for(int t = 1; t <= T; t++)
    {
        double chosenReserve = bot.estimateCriticalValue();
        // printf("Chosen Reserve: %.4lf\n", chosenReserve);

        vector<double> b(bidders);
        for(int i = 0; i < bidders; i++) b[i] = F();

        // Calculate Revenue
        vector<double> tmp(1, chosenReserve);
        double revenueEarned = getAuctionRevenue(k, b, tmp)[0];
        totalRevenue += revenueEarned;


        for(double v: b) bot.updateValue(v);
    }

    printf("Average Revenue: %.5lf\n", totalRevenue / T);
    printf("Final Reserve Estimate: %.5lf\n", bot.estimateCriticalValue());
}




int main()
{
    vector<double> rsv(100); for(int i = 0; i < 100; i++) rsv[i] = 0.01 * i;
    // double OPT = getExpectedRevenue(1, 3, 1.0 / sqrt(3.0), mystery1, 1000000);
    // double OPT = getExpectedRevenue(1, 3, 0.5, mystery1, 1000000);
    // printf("OPT: %.4lf\n", OPT);
    // auctionEW(1, 3, 100000, 0.1, rsv, mystery2);
    auctionEmpirical(1, 2, 1000, mystery1);
    // tradeEW(10000, 1, rsv, uniform, uniform);

    return 0;
}
