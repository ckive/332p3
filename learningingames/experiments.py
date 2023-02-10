from agent import OnlineLearner
from environment import BiGameEnv
from games import Bimatrix3x3
import numpy as np
from tqdm import tqdm

# Add Monte Carlo using this infrastructure

# Example game: eps0 vs eps0.5 on avoidtie game

games = Bimatrix3x3()

# NOTE, N = 1000, theo eps = 0.003314
T = 10
N = 1000
theo_eps = np.sqrt(np.log(3)/N)
epsrange = sorted([0,10000,theo_eps, theo_eps/2, 2*theo_eps, theo_eps/5, 5*theo_eps, theo_eps/10, 10*theo_eps])

# # eps0learner = OnlineLearner(games.avoidties, 0.001)
# # eps05learner = OnlineLearner(games.avoidties, 0.001)
# # env = BiGameEnv([eps0learner, eps05learner], games.avoidties)
# eps0learner = OnlineLearner(games.g3, 100000)
# eps05learner = OnlineLearner(games.g3, 100000)
# env = BiGameEnv([eps0learner, eps05learner], games.g3)
# actions, scoreboard = env.run(1000)
# # print(actions, scoreboard)
# print(scoreboard.sum(axis=1))
# print('##################')
# # env.show_player_weights()
# env.analyze_actions()
# print('##################')


payoff_mew1 = np.zeros((3,9,9))
payoff_mew2 = np.zeros((3,9,9))
payoff_std1 = np.zeros((3,9,9))
payoff_std2 = np.zeros((3,9,9))

def runmc(p1, p2, T, N):
    for t in tqdm(range(T)):
    # for t in range(T):
        # play game
        env = BiGameEnv([player1, player2], games.g3)
        actions, scoreboard = env.run(N)
        regrets = env.analyze_history()
        montecarlo_collector_payoffs[t] = scoreboard.sum(axis=1)

# outter most loop
# for g, game in enumerate(tqdm([games.avoidties, games.loveties, games.g3])):
for g, game in enumerate([games.avoidties, games.loveties, games.g3]):
    g = 0; game=games.g3
    # play the 3rd game

    for e1, eps1 in enumerate(epsrange):
        # eps1 = theo_eps
        player1 = OnlineLearner(game, eps1)
        for e2, eps2 in enumerate(epsrange):
            # eps2 = theo_eps
            player2 = OnlineLearner(game, eps2)
            montecarlo_collector_payoffs = np.zeros((T,2))
            print('ahhh', eps1, eps2, T, N)
            runmc(eps1, eps2, T, N)
            # fill buckets from mcp
            payoff_mew1[g][e1][e2], payoff_mew2[g][e1][e2] = montecarlo_collector_payoffs.mean(axis=0)/T
            payoff_std1[g][e1][e2], payoff_std2[g][e1][e2] = montecarlo_collector_payoffs.std(axis=0)/T
            # break
        # break
    # running just 1 game
    break

print('hi')

# calc avg, std from collector, store in buckets

