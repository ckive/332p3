
# \begin{tabular}{@{}c@{}}Three \\ Drie\end{tabular}

import random as rnd

def getLatexTable(m1, m2, s1, s2, eps):
    # Assume that all is k * k matrix
    k = len(eps)
    s = "\\begin{tabular}{|c||%s}" % ("c|" * (k))
    s += "\n"
    # Header Row
    s += "\\hline\n"
    s += "$\\varepsilon$"
    for e in eps:
        s += "& $%.2f$ " % e
    s += "\\\\\n"
    s += "\\hline\\hline\n"
    # Other Rows
    for i in range(k):
        # Label
        s += "$%.2f$ " % (eps[i])
        # Table Entries
        for j in range(k):
            s += " & \\begin{tabular}{@{}c@{}} \\textcolor{red}{$%.1f \\pm %.1f$} \\\\ \\textcolor{blue}{$%.1f \\pm %.1f$} \\end{tabular}" % (m1[i][j], s1[i][j], m2[i][j], s2[i][j])
        s += " \\\\\n"
        s += "\\hline\n"



    s += "\\end{tabular}"
    return s


if __name__ == "__main__":

    K = 9
    a1 = [[rnd.randint(0,99) / 10 for j in range(K)] for i in range(K)]
    a2 = [[rnd.randint(0,99) / 10 for j in range(K)] for i in range(K)]
    a3 = [[rnd.randint(0,99) / 10 for j in range(K)] for i in range(K)]
    a4 = [[rnd.randint(0,99) / 10 for j in range(K)] for i in range(K)]
    b = [rnd.randint(0,99) / 10 for j in range(K)]

    testMu = [[1, 2], [3, 4]]
    testSigma = [[5, 6], [7, 8]]
    eps = [0.01, 0.02]

    print(getLatexTable(a1, a2, a3, a4, b))