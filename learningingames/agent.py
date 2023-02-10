import numpy as np


class OnlineLearner:
    """
    Is a learner that plays exponential weights
    """
    def __init__(self, game, eps=0):
        self.eps = eps
        self.game = game
        self.gamesize = game.shape[-1]
        self.weights = np.array([1/self.gamesize]*self.gamesize)
        


    def play(self, exploit_eps=False):
        """
        performs ew on 1 round's payoff matrix
        returns action
        """
        if exploit_eps:
            # we have eps of the other player knowing it plays exponential weights
            raise NotImplementedError
        else:
            # chooses action based on weights in exponential weights
            a_j = np.random.choice(self.gamesize, p=self.weights)
            return a_j
    
    # def recv_feedback(self, action, payoff):
    #     # update weights step           (note, 1+eps i think jason typo)
    #     self.weights[action] = self.weights[action] * np.power(1+self.eps, payoff)
    #     # normalize
    #     wsum = sum(self.weights)
    #     self.weights = self.weights/wsum
    #     wsum = 1

    def recv_feedback(self, payoffs):
        # update weights step           (note, 1+eps i think jason typo)
        self.weights = self.weights * np.float_power(1+self.eps, payoffs)
        # normalize
        wsum = sum(self.weights)
        self.weights = self.weights/wsum
        a = 1
