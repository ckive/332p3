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
        self.N = N
        J = len(self.players)
        self.playeractions = np.zeros((J,N), dtype=int)
        self.scoreboard = np.zeros((J,N))
        self.cfpayoffs = np.zeros((J,N,3))
        
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

            payoffs_row_sees = np.array(list(zip(*self.game[0]))[col_a])
            payoffs_col_sees = self.game[1][row_a]

            self.players[0].recv_feedback(payoffs_row_sees)
            self.players[1].recv_feedback(payoffs_col_sees)

            # store in cf
            self.cfpayoffs[0][i] = payoffs_row_sees
            self.cfpayoffs[1][i] = payoffs_col_sees
        
        return self.playeractions, self.scoreboard

    def analyze_actions(self):
        # go thru each of the action pairs and count occurence 
        for i in range(self.N):
            action_pair = (self.playeractions[0][i], self.playeractions[1][i])
            self.actionpaircounts[action_pair] += 1
        # print(self.actionpaircounts)
        for ac_pair, counts in self.actionpaircounts.items():
            print(f"Action Pair: {ac_pair} played {round((counts/self.N)*100, 3)}% times")
        return self.actionpaircounts

    def analyze_history(self, draw=False):
        # calculates regret/round

        cumcfpayoffs = self.cfpayoffs.cumsum(axis=1)
        bih_payoffs_each_round = np.max(cumcfpayoffs, axis=2)

        actualpayoffs_each_round = self.scoreboard.cumsum(axis=1)
        regret_per_round = (bih_payoffs_each_round - actualpayoffs_each_round) / list(range(1,self.N+1))
        return regret_per_round

