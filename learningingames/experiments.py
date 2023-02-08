from agent import OnlineLearner
from environment import BiGameEnv
from games import Bimatrix3x3
import numpy as np

# Add Monte Carlo using this infrastructure

# Example game: eps0 vs eps0.5 on avoidtie game

games = Bimatrix3x3()

eps0learner = OnlineLearner(games.at_base, 2)
eps05learner = OnlineLearner(games.at_base, 2)
env = BiGameEnv([eps0learner, eps05learner], games.at_base)
actions, scoreboard = env.run(100)
# print(actions, scoreboard)
print(scoreboard.sum(axis=1))
print('##################')
# env.show_player_weights()
env.analyze_actions()
print('##################')
# for acsj in actions:
#     print(np.unique(acsj, return_counts=True))
