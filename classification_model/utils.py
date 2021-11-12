from pycsca.constants import *


cols_base = ['Dataset', 'Model']
cols_metrics = [ACCURACY, '{}-std'.format(ACCURACY), F1SCORE, "{}-std".format(F1SCORE),
                AUC_SCORE, "{}-std".format(AUC_SCORE), COHENKAPPA, "{}-std".format(COHENKAPPA), MCC,
                "{}-std".format(MCC), INFORMEDNESS, "{}-std".format(INFORMEDNESS)]
cols_pvals = [FISHER_PVAL + '-single', FISHER_PVAL + '-sum', FISHER_PVAL + '-median', FISHER_PVAL + '-mean',
              FISHER_PVAL + '-holm-bonferroni', CTTEST_PVAL + '-random', CTTEST_PVAL + '-majority',
              CTTEST_PVAL + '-prior', TTEST_PVAL + '-random', TTEST_PVAL + '-majority', TTEST_PVAL + '-prior',
              WILCOXON_PVAL + '-random', WILCOXON_PVAL + '-majority', WILCOXON_PVAL + '-prior']
columns = cols_base + cols_metrics + cols_pvals
test_size = 0.3
