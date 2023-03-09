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


class SimpleFirstPriceExponentialAgent
{
private:
    double v; // Value if win
    double eps; // Learning Rate
    vector<double> bidChoices;  // Assume this is increasing
    vector<double> w;

public:
    SimpleFirstPriceExponentialAgent(double value, double EPS, const vector<double> &BID_CHOICES)
    {
        v = value;
        eps = EPS;
        bidChoices = BID_CHOICES;
        w = vector<double>(bidChoices.size(), 1);
        normalizeProbability(w);
    }

    double getBid()
    {
        int idx = getRandomAction(w);
        return bidChoices[idx];
    }

    void updateWeight(double criticalBid)
    {
        const int m = bidChoices.size();
        for(int i = m - 1; i >= 0; i--)
        {
            if(bidChoices[i] < criticalBid) break;  // Payoff will be 0 for those cases
            const double payoff = v - bidChoices[i];
            w[i] *= pow(1.0 + eps, payoff / v); // Here, h = v is the maximum payoff
        }
        normalizeProbability(w);
    }
};

pair<pair<vector<vector<double>>, vector<vector<double>>>, vector<int>> qualityWeightFirstPriceAuction(vector<SimpleFirstPriceExponentialAgent> &agents, int T)
{
    const int n = agents.size();    // The number of bidders
    vector<int> winnerList;
    vector<vector<double>> allBids;
    vector<vector<double>> allQ;
    for(int t = 1; t <= T; t++)
    {
        vector<double> bids(n); for(int i = 0; i < n; i++) bids[i] = agents[i].getBid();
        vector<double> q(n); for(int i = 0; i < n; i++) q[i] = myRandom(0, 1);

        double winVal = -1;
        int winId = -1;
        for(int i = 0; i < n; i++)
        {
            if(bids[i] * q[i] >= winVal)    // Here, Pr(tie) = 0 so that we can do tiebreak however we want!
            {
                winVal = bids[i] * q[i];
                winId = i;
            }
        }

        winnerList.push_back(winId);
        allBids.push_back(bids);
        allQ.push_back(q);

        // Winner's weighted bid is the critical bid for the losers

        // Find Critical (weighted) Bid for the winner
        double secondPlace = 0.0;
        for(int i = 0; i < n; i++) if(i != winId) secondPlace = max(secondPlace, bids[i] * q[i]);

        // Send Update to Agents
        for(int i = 0; i < n; i++)
        {
            if(i == winId) agents[i].updateWeight(secondPlace / q[i]);
            else agents[i].updateWeight(winVal / q[i]);
        }
    }
    return {{allBids, allQ}, winnerList};
}

vector<vector<double>> getRegret(int T, const vector<double> &possibleValues, const vector<vector<double>> &q, const vector<vector<double>> &b, const vector<int> winners)
{
    const int n = q[0].size();  // Number of Bidders
    const int m = possibleValues.size();    // Number of possible values we are considering
    // return: n x m vectors regret where regret[i][j] is the regret of agent i if his value is possibleValues[j]

    // Pre-Process: Calculate Critical Weighted Bid
    vector<double> firstPlace(T, 0.0);
    vector<double> secondPlace(T, 0.0);
    for(int t = 0; t < T; t++)
    {
        firstPlace[t] = q[t][winners[t]] * b[t][winners[t]];
        for(int i = 0; i < n; i++) if(i != winners[t]) secondPlace[t] = max(secondPlace[t], q[t][i] * b[t][i]);

        // fprintf(stderr,"Weighted Bid: "); for(int i = 0; i < n; i++) fprintf(stderr,"%.4lf ", q[t][i] * b[t][i]); fprintf(stderr," (Winner: %d)\n", winners[t] + 1);

        // fprintf(stderr,"Round %d: %.3lf, %.3lf\n", t + 1, firstPlace[t], secondPlace[t]);
    }

    vector<vector<double>> allRegret;
    for(int i = 0; i < n; i++)
    {
        // fprintf(stderr,"Agent %d\n", i);
        double actualPayment = 0.0;
        double actualWinCount = 0.0;

        vector<double> cb(T);
        for(int t = 0; t < T; t++)
        {
            if(winners[t] == i)
            {
                cb[t] = secondPlace[t] / q[t][i];
                actualPayment += b[t][i];
                actualWinCount += 1.0;
            }
            else
            {
                cb[t] = firstPlace[t] / q[t][i];
            }
        }

        // fprintf(stderr,"Win Count: %.0lf (Payment: %.3lf)\n", actualWinCount, actualPayment);

        // Notice that best in hindsight would be a critical bid in some round (or zero)
        sort(cb.begin(), cb.end());

        vector<double> OPT(m, 0.0);
        for(int j = 0; j < m; j++)
        {
            for(int t = 0; t < T; t++)
            {
                double winProb = ((double) t + 1.0) / ((double) T);
                double utilityIfWin = possibleValues[j] - cb[t];
                OPT[j] = max(OPT[j], winProb * utilityIfWin);
            }
        }

        // Calculate Regret
        vector<double> regret(m);
        for(int j = 0; j < m; j++)
        {
            double payoff = actualWinCount * possibleValues[j] - actualPayment;
            payoff /= (double) T;

            regret[j] = OPT[j] - payoff;
        }

        allRegret.push_back(regret);
    }
    return allRegret;
}

vector<double> guessValue(const vector<double> &possibleValues, const vector<vector<double>> &regret)
{
    const int n = regret.size(); // Number of Bidders
    const int m = possibleValues.size(); // Number of possible values we are considering
    // return array v of length n where v[i] is guessed value of agent[i]

    vector<double> v;
    for(int i = 0; i < n; i++)
    {
        double lowest = 999999999;
        double curArg = 0.0;
        for(int j = 0; j < m; j++)
        {
            if(regret[i][j] <= lowest)
            {
                curArg = possibleValues[j];
                lowest = regret[i][j];
            }
        }

        v.push_back(curArg);
    }
    return v;
}

int main()
{
    vector<double> possibleBid; for(int i = 0; i < 20; i++) possibleBid.push_back(0.05 * i);
    vector<SimpleFirstPriceExponentialAgent> agents;
    agents.push_back(SimpleFirstPriceExponentialAgent(0.6, 0.1, possibleBid));
    agents.push_back(SimpleFirstPriceExponentialAgent(0.7, 0.1, possibleBid));
    agents.push_back(SimpleFirstPriceExponentialAgent(0.8, 0.1, possibleBid));
    agents.push_back(SimpleFirstPriceExponentialAgent(0.9, 0.1, possibleBid));

    pair<pair<vector<vector<double>>, vector<vector<double>>>, vector<int>> auctionLog = qualityWeightFirstPriceAuction(agents, 100000);

    // Inference
    vector<double> possibleValues; for(int i = 0; i < 100; i++) possibleValues.push_back(0.01 * i);

    vector<vector<double>> regret = getRegret(1000, possibleValues, auctionLog.first.second, auctionLog.first.first, auctionLog.second);

    if(false)
    {
        fprintf(stderr,"Regret\n");
        for(int i = 0; i < regret.size(); i++)
        {
            for(int j = 0; j < possibleValues.size(); j++) fprintf(stderr,"%.3lf ", regret[i][j]);
            fprintf(stderr,"\n");
        }
    }

    vector<double> guess = guessValue(possibleValues, regret);
    printf("Guessed Value:\n");
    for(double v: guess) printf("%.2lf ", v);
    printf("\n");

    return 0;
}
