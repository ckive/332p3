from agent import OnlineLearner
from environment import BiGameEnv
from games import Bimatrix3x3
import numpy as np

# Add Monte Carlo using this infrastructure

# Example game: eps0 vs eps0.5 on avoidtie game

games = Bimatrix3x3()

# NOTE, N = 1000, theo eps = 0.003314
T = 10
N = 1000
theo_eps = np.sqrt(np.log(3)/N)
epsrange = sorted([0,np.inf,theo_eps, theo_eps/2, 2*theo_eps, theo_eps/5, 5*theo_eps, theo_eps/10, 10*theo_eps])

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


regret_mew1 = np.zeros((3,9,9))
regret_mew2 = np.zeros((3,9,9))
regret_std1 = np.zeros((3,9,9))
regret_std2 = np.zeros((3,9,9))

# outter most loop
# for g, game in enumerate([games.avoidties, games.loveties, games.g3]):
#     pass

montecarlo_collector_payoffs = np.zeros((T,2))

# monteCarlo 200 times
for t in range(T):
    # play the 3rd game
    for eps1 in epsrange:
        eps1 = theo_eps
        player1 = OnlineLearner(games.g3, eps1)
        for eps2 in epsrange:
            eps2 = theo_eps
            player2 = OnlineLearner(games.g3, eps2)
            # play game
            env = BiGameEnv([player1, player2], games.g3)
            actions, scoreboard = env.run(N)
            regrets = env.analyze_history()
            montecarlo_collector_payoffs[t] = scoreboard.sum(axis=1)

print('hi')

# calc avg, std from collector, store in buckets

