#include<bits/stdc++.h>
using namespace std;

mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
double myRandom(double L, double R)
{
    uniform_real_distribution<> work(L,R);
    return work(rng);
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


class OrderingExponentialAgent
{
private:
    double v; // Value if win
    double eps; // Learning Rate
    vector<double> bidChoices;  // Assume this is increasing
    vector<double> w;

public:
    OrderingExponentialAgent(double value, double EPS, const vector<double> &BID_CHOICES)
    {
        v = value;
        eps = EPS;
        bidChoices = BID_CHOICES;
        w = vector<double>(bidChoices.size(), 1);
        normalizeProbability(w);
    }

    int getBidIndex()
    {
        return getRandomAction(w);
    }

    void updateWeight(vector<double> slots)
    {
        const int m = bidChoices.size();
        for(int i = m - 1; i >= 0; i--)
        {
            if(slots[i] <= 0) break;
            const double payoff = slots[i] * (v - bidChoices[i]);
            w[i] *= pow(1.0 + eps, payoff / v); // Here, h = v is the maximum payoff
        }
        normalizeProbability(w);
    }
};

double getCoinProb(int agentId, int newBid, const vector<int> &oldBid, const vector<double> &prefixSum)
{
    int loseCount = 0;
    int tieCount = 0;
    for(int i = 0; i < oldBid.size(); i++)
    {
        if(i == agentId) continue;
        if(newBid < oldBid[i]) loseCount++;
        else if(newBid == oldBid[i]) tieCount++;
    }
    double winning = prefixSum[loseCount + tieCount];
    if(loseCount > 0) winning -= prefixSum[loseCount - 1];
    winning /= (1.0 + tieCount);
    return winning;
}

pair< pair<vector<double>,vector<double>>, vector<vector<double>>> orderingAuction(vector<OrderingExponentialAgent> &agents, const vector<double> &slotProb, const vector<double> &possibleBids, int T, const double minimumBet)
{
    const int n = agents.size();
    const int m = possibleBids.size();

    // Preprocess: Prefix sum
    vector<double> ps(m); ps[0] = slotProb[0];
    for(int i = 1; i < m; i++) ps[i] = ps[i - 1] + slotProb[i];

    vector<double> actualWin(n);
    vector<double> actualPay(n);
    vector<vector<double>> oneActionWinCount(n, vector<double>(m));

    for(int t = 1; t <= T; t++)
    {
        vector<int> bidIndices(n);
        for(int i = 0; i < n; i++) bidIndices[i] = agents[i].getBidIndex();

        // Calculate Actual Payoff
        for(int i = 0; i < n; i++)
        {
            if(possibleBids[bidIndices[i]] < minimumBet) continue;  // Does not count
            double w = getCoinProb(i, bidIndices[i], bidIndices, ps);
            actualWin[i] += w;
            actualPay[i] += w * possibleBids[bidIndices[i]];
        }

        // Calculate Hypothetical Payoff (for exponential weight)
        for(int i = 0; i < n; i++)
        {
            vector<double> coinProb(m, 0.0);
            for(int j = m - 1; j >= 0; j--)
            {
                if(possibleBids[j] < minimumBet) break;  // Below Reserve -> 0
                coinProb[j] = getCoinProb(i, j, bidIndices, ps);
            }

            // Update Weight
            agents[i].updateWeight(coinProb);

            // Contribute to best in hindsight
            for(int j = 0; j < m; j++) oneActionWinCount[i][j] += coinProb[j];

        }
    }

    // Calculate Average of Everything
    for(int i = 0; i < n; i++) actualWin[i] /= (double) T;
    for(int i = 0; i < n; i++) actualPay[i] /= (double) T;
    for(int i = 0; i < n; i++) for(int j = 0; j < m; j++) oneActionWinCount[i][j] /= (double) T;

    return {{actualWin, actualPay}, oneActionWinCount};
}

vector<double> guessValue(const vector<double> &actualWin, const vector<double> &actualPay, const vector<vector<double>> &oneAction, const vector<double> &possibleBid, const vector<double> &possibleValues)
{
    const int n = actualWin.size();
    const int m = possibleBid.size();
    vector<double> guess(n);
    for(int i = 0; i < n; i++)
    {
        double bestArgument = 0.0;
        double bestRegret = 999999999.332;

        for(double v: possibleValues)
        {
            double OPT = 0.0;
            for(int j = 0; j < m; j++) OPT = max(OPT, oneAction[i][j] * (v - possibleBid[j]));

            double ALG = actualWin[i] * v - actualPay[i];
            double regret = OPT - ALG;

            if(regret < bestRegret)
            {
                bestRegret = regret;
                bestArgument = v;
            }
        }

        guess[i] = bestArgument;
    }
    return guess;
}

double getSum(const vector<double> &a)
{
    double ret = 0.0;
    for(double v: a) ret += v;
    return ret;
}

double revenueEvaluation(const vector<double> values, const vector<double> &slots, const double reservePrice, int T)
{
    vector<double> possibleBid; for(int i = 0; i < 20; i++) possibleBid.push_back(0.05 * i);
    vector<OrderingExponentialAgent> agents;
    for(double v: values) agents.push_back(OrderingExponentialAgent(v, 0.1, possibleBid));
    auto auctionLog = orderingAuction(agents, slots, possibleBid, T, reservePrice);
    return getSum(auctionLog.first.second);
}

double reserveSearch(const vector<double> values, const vector<double> &slots, const vector<double> &reserveCandidates)
{
    const int T = 100000;
    double bestRevenue = 0.0;
    double bestReserve = 0.0;
    for(double r: reserveCandidates)
    {
        const double revenue = revenueEvaluation(values, slots, r, T);
        if(revenue > bestRevenue)
        {
            bestRevenue = revenue;
            bestReserve = r;
        }
    }
    return bestReserve;
}

void partTwoExperiment(const vector<double> realValues, const vector<double> slots, const double EPS = 0.1)
{
    const int n = realValues.size();
    vector<double> possibleBid; for(int i = 0; i < 20; i++) possibleBid.push_back(0.05 * i);
    vector<OrderingExponentialAgent> agents;
    for(double v: realValues) agents.push_back(OrderingExponentialAgent(v, EPS, possibleBid));

    pair< pair<vector<double>,vector<double>>, vector<vector<double>>> auctionLog = orderingAuction(agents, slots, possibleBid, 100000, 0.0);

    vector<double> winProb = auctionLog.first.first;
    // fprintf(stderr,"winProb: "); for(int i = 0; i < n; i++) fprintf(stderr,"%.4lf ", winProb[i]); fprintf(stderr,"\n");
    vector<double> payment = auctionLog.first.second;
    // fprintf(stderr,"payment: "); for(int i = 0; i < n; i++) fprintf(stderr,"%.4lf ", payment[i]); fprintf(stderr,"\n");



    const double R1 = getSum(auctionLog.first.second);
    printf("Base Revenue: %.4lf\n", R1);

    vector<double> possibleValues(100); for(int i = 0; i < 100; i++) possibleValues.push_back(0.01 * i);

    vector<double> guess = guessValue(auctionLog.first.first, auctionLog.first.second, auctionLog.second, possibleBid, possibleValues);
    printf("Guess: "); for(int i = 0; i < n; i++) printf("%.2lf ", guess[i]); printf("\n");

    double r_OPT = reserveSearch(realValues, slots, possibleBid);
    printf("Optimal Reserve: %.4lf\n", r_OPT);

    double r_guess = reserveSearch(guess, slots, possibleBid);
    printf("Tuned Reserve: %.4lf\n", r_guess);

    double rev_OPT = revenueEvaluation(realValues, slots, r_OPT, 1000000);
    double rev_ALG = revenueEvaluation(realValues, slots, r_guess, 1000000);

    printf("Optimal Revenue: %.4lf\n", rev_OPT);
    printf("Tuned Revenue: %.4lf\n", rev_ALG);
}

int main()
{
    // Auction
    const vector<double> slots = {1.0, 0.8, 0.4};
    const vector<double> realValues = {0.6, 0.7, 0.8};
    partTwoExperiment(realValues, slots, 0.1);
    return 0;
}

