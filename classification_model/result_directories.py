import logging

import os
from pycsca.utils import create_dir_recursively
import shutil


class ResultDirectories():
    def __init__(self, folder: str, debug_level=0):
        self.folder = folder
        self.debug_folder = os.path.join(self.folder, "Debug Results")
        self.intermediate_folder = os.path.join(self.folder, "Intermediate Results")
        self.final_results_folder = os.path.join(self.folder, "Final Results")
        self.debug_level = debug_level
        self._create_directories_()
        self.logger = logging.getLogger(ResultDirectories.__name__)

    def _create_directories_(self):
        self._create_intermediate_folders_()
        self._create_debug_folders_()
        self._create_final_folders_()

    def _create_intermediate_folders_(self):
        self.accuracies_file = os.path.join(self.folder, self.intermediate_folder, 'Model Accuracies.pickle')
        self.vulnerable_file = os.path.join(self.folder, self.intermediate_folder, 'Vulnerable Classes.pickle')
        self.model_result_file_path = os.path.join(self.folder, self.intermediate_folder, 'Model Results.csv')
        self.models_folder = os.path.join(self.folder, self.intermediate_folder, 'Models')
        self.result_file_path = os.path.join(self.folder, self.intermediate_folder, 'Final Results.csv')
        self.detailed_report_file = os.path.join(self.folder, self.intermediate_folder, 'Detailed Report.txt')
        create_dir_recursively(self.models_folder, False)

    def _create_debug_folders_(self):
        self.learning_log_file = os.path.join(self.folder, self.debug_folder, 'learning.log')
        self.plotting_log_file = os.path.join(self.folder, self.debug_folder, 'plotting.log')
        self.pvalue_cal_log_file = os.path.join(self.folder, self.debug_folder, 'p-value-calculation.log')
        create_dir_recursively(self.learning_log_file, True)


    def _create_final_folders_(self):
        self.plots_folder = os.path.join(self.folder, self.final_results_folder, 'Plots')
        self.importance_folder = os.path.join(self.plots_folder, 'Feature Importance')
        self.learning_curves_folder = os.path.join(self.plots_folder, 'Learning Curves')
        self.report_file = os.path.join(self.folder, self.final_results_folder, 'Report.txt')
        create_dir_recursively(self.plots_folder, False)
        create_dir_recursively(self.learning_curves_folder, False)
        create_dir_recursively(self.importance_folder, False)

    def remove_folders(self):
        if self.debug_level < 2:
            shutil.rmtree(self.debug_folder)
        if self.debug_level == 0:
            shutil.rmtree(self.intermediate_folder)
