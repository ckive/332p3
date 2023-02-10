import numpy as np
import nashpy as nash
class Bimatrix3x3:
    """
    Nx2x3x3 arrays where 1st dim is row(0) and col(1) player payoffs and for N rounds
    """
    def __init__(self):
        # fixed
        self.at_base = np.array(
            [
                [-6,-1,1],
                [1,-6,-1],
                [-1,1,-6]
            ]
        )
        self.avoidties = np.array([self.at_base, self.at_base.transpose()])
        self.atgame = nash.Game(self.at_base)
        self.atg_xnash = self.atgame.support_enumeration()
        
        self.lt_base = np.array(
            [
                [-6,-1,1],
                [1,-6,-1],
                [-1,1,6]
            ]
        )
        self.loveties = np.array([self.lt_base, self.lt_base.transpose()])
        self.ltgame = nash.Game(self.lt_base)
        self.ltg_xnash = self.ltgame.support_enumeration()

        # can change
        self.gen_mystery()

        # third game
        self.third_base = np.array(
            [
                [-6,-1,1],
                [1,-6,-1],
                [-1,1,6]
            ]
        )
        self.loveties = np.array([self.third_base, self.third_base.transpose()])
        self.ltgame = nash.Game(self.third_base)
        self.ltg_xnash = self.ltgame.support_enumeration()


    def gen_mystery(self):
        self.mystery_base = np.random.randint(-10,10,(3,3))
        self.mysterygame = np.array([self.mystery_base, self.mystery_base.transpose()])
        mysgame = nash.Game(self.mystery_base)
        mysg_xnash = mysgame.support_enumeration()
    
