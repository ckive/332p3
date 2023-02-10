from agent import OnlineLearner
from typing import List
import numpy as np
from itertools import product

class BiGameEnv:
    def __init__(self, players: List[OnlineLearner], game):
        # note, the game passed in is the full 2player game
        self.players = players
        # self.gamebase = game
        self.game = game
        gameN = game[-1].shape[-1]
        self.actionpaircounts = {ap: 0 for ap in product(range(gameN),range(gameN))}
        self.playerconclusions = []
        
    def run(self, N):
        # runs the game for N rounds, players play simultaneously
        # coordinates payoffs for agents in round i
        J = len(self.players)
        self.playeractions = np.zeros((J,N), dtype=int)
        self.scoreboard = np.zeros((J,N))
        
        for i in range(N):
            # perform action simultaneously for each agent
            for j in range(J):
                a_j = self.players[j].play()
                self.playeractions[j][i] = a_j
            # coordinate payoffs and update agents' weights
            row_a = self.playeractions[0][i]
            col_a = self.playeractions[1][i]

            self.scoreboard[0][i] = self.game[0][row_a][col_a]
            self.scoreboard[1][i] = self.game[1][row_a][col_a]

            self.players[0].recv_feedback(list(zip(*self.game[0]))[col_a])
            self.players[1].recv_feedback(list(zip(*self.game[1]))[row_a])

            # for j in range(J):
            #     me = self.playeractions[j][i]
            #     them = self.playeractions[j^1][i]
            #     # self.scoreboard[j][i] = self.gamebase[me][them]

            #     self.scoreboard[j][i] = self.game[j^1][them][me]
            #     # self.players[j].recv_feedback(me, self.scoreboard[j][i])
            #     # my payoff is the opponent's action's possible payoffs
            #     self.players[j].recv_feedback(self.game[j^1][them])
        
        # player conclusions
        for j in range(J):
            self.players[j].conclude()

        
        return self.playeractions, self.scoreboard

    def analyze_actions(self):
        # go thru each of the action pairs and count occurence 
        n = len(self.playeractions[0])
        for i in range(n):
            action_pair = (self.playeractions[0][i], self.playeractions[1][i])
            self.actionpaircounts[action_pair] += 1
        # print(self.actionpaircounts)
        for ac_pair, counts in self.actionpaircounts.items():
            print(f"Action Pair: {ac_pair} played {round((counts/n)*100, 3)}% times")
        return self.actionpaircounts

    def analyze_history(self, draw=False):
        # calculates regret/round and optionally plots
        rowP = self.players[0]
        colP = self.players[1]
