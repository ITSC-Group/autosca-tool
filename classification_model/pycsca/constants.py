INFORMEDNESS = "Informedness"
MCC = "MathewsCorrelationCoefficient"
COHENKAPPA = "Cohen-Kappa"
ACCURACY = 'Accuracy'
F1SCORE = 'F1-Score'
AUC_SCORE = 'AUC-Score'
CONFUSION_MATRIX_SINGLE = 'confusion_matrix_single'
CONFUSION_MATRICES = 'confusion_matrices'
METRICS = [ACCURACY, F1SCORE, AUC_SCORE, COHENKAPPA, MCC, INFORMEDNESS]
BEST_PARAMETERS = 'Best-Parameters'
TIME_TAKEN = 'HPO-Time'
MODEL = 'Model'
FOLD_ID = 'Fold-ID'
DATASET = 'Dataset'
MULTI_CLASS = 'Multi-Class'
TTEST_PVAL = "ttest-pval"
CTTEST_PVAL = "corrected-ttest-pval"
CV_ITERATIONS_LABEL = "CV-ITERATIONS"
WILCOXON_PVAL = 'wilcoxon-pval'
FISHER_PVAL = 'fisher-pval'
SCORE_KEY_FORMAT = '{}-scores-{}'

LABEL_COL = 'label'
MISSING_CCS_FIN = 'missing_ccs_fin'
cv_choices = ['kfcv', 'mccv', 'auto']
debug_levels = {0: "Final", 1: "Intermediate", 2: "Debug"}
CV_ITERATOR = "CV_ITERATOR"
N_SPLITS = "N_SPLITS"
DEBUG_LEVEL = "DEBUG_LEVEL"
LOW = "LOW"
MEDIUM = "MEDIUM"
HIGH = "HIGH"
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
P_VALUE_COLUMN = FISHER_PVAL + '-median'
