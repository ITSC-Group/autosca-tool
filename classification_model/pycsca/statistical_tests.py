import logging

import numpy as np
from scipy.stats import t, wilcoxon

__all__ = ["wilcoxon_signed_rank_test", "paired_ttest"]
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

def wilcoxon_signed_rank_test(accuracies, accuracies2, alternative="two-sided"):
    logger = logging.getLogger('Wilcoxon-Signed_Rank')

    try:
        _, p_value = wilcoxon(accuracies, accuracies2, correction=True, alternative=alternative)
    except Exception as e:
        logger.info('Accuracies are exactly same {}'.format(str(e)))
        p_value = 1.0
    return p_value


def paired_ttest(x1, x2, n_training_folds, n_test_folds, correction=False, alternative="two-sided"):
    n = len(x1)
    df = n - 1
    diff = [(x1[i] - x2[i]) for i in range(n)]
    # Compute the mean of differences
    d_bar = np.mean(diff)
    # compute the variance of differences
    sigma2 = np.var(diff, ddof=1)

    logger = logging.getLogger('Paired T-Test')
    if correction:
        logger.info("With the correction option")
    logger.info("D_bar {} Variance {} Sigma {}".format(d_bar, sigma2, np.sqrt(sigma2)))

    # compute the modified variance
    if correction:
        sigma2 = sigma2 * (1 / n + n_test_folds / n_training_folds)
    else:
        sigma2 = sigma2 / n

    # compute the t_static
    with np.errstate(divide='ignore', invalid='ignore'):
        t_static = np.divide(d_bar, np.sqrt(sigma2))

    # Compute p-value and plot the results
    if alternative == 'less':
        p = t.cdf(t_static, df)
    elif alternative == 'greater':
        p = t.sf(t_static, df)
    elif alternative == 'two-sided':
        p = 2 * t.sf(np.abs(t_static), df)

    logger.info("Final Variance {} Sigma {} t_static {} p {}".format(sigma2, np.sqrt(sigma2), t_static, p))
    logger.info("np.isnan(p) {}, np.isinf {},  d_bar == 0 {}, sigma2_mod == 0 {}, np.isinf(t_static) {}, "
                "np.isnan(t_static) {}".format(np.isnan(p), np.isinf(p), d_bar == 0, sigma2 == 0, np.isinf(t_static),
                                               np.isnan(t_static)))
    if np.isnan(p) or np.isinf(p) or d_bar == 0 or sigma2 == 0 or np.isinf(t_static) or np.isnan(t_static):
        p = 1.0
    return p
