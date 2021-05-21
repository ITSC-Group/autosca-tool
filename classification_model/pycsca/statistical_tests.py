import numpy as np
from scipy.stats import t


# def corrected_dependent_ttest(x1, x2, n_training_folds, n_test_folds, alpha):
#     n = len(x1)
#     differences = [(x1[i] - x2[i]) for i in range(n)]
#     sd = stdev(differences)
#     divisor = 1 / n * sum(differences)
#     test_training_ratio = n_test_folds / n_training_folds
#     denominator = sqrt(1 / n + test_training_ratio) * sd
#     t_stat = divisor / denominator
#     # degrees of freedom
#     df = n - 1
#     # calculate the critical value
#     cv = t.ppf(1.0 - alpha, df)
#     # calculate the p-value
#     p = (1.0 - t.cdf(abs(t_stat), df)) * 2.0
#     # return everything
#     return t_stat, df, cv, p


def corrected_dependent_ttest(x1, x2, n_training_folds, n_test_folds, alpha):
    n = len(x1)
    diff = [(x1[i] - x2[i]) for i in range(n)]
    # Compute the mean of differences
    d_bar = np.mean(diff)
    # compute the variance of differences
    sigma2 = np.var(diff, ddof=1)
    # compute the modified variance
    sigma2_mod = sigma2 * (1 / n + n_test_folds / n_training_folds)
    # compute the t_static
    t_static = d_bar / np.sqrt(sigma2_mod)
    # Compute p-value and plot the results
    p = 2 * t.sf(np.abs(t_static), n - 1)
    # p = (1 - t.cdf(abs(t_static), n - 1)) * 2
    # cv = t.ppf(1.0 - alpha, n - 1)
    if np.isnan(p) or d_bar == 0:
        p = 1.0
    return p
