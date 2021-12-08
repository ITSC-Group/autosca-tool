import os
from pycsca.utils import create_dir_recursively


class ResultDirectories():
    def __init__(self, folder: str, debug_level=0):
        self.debug_folder = "Debug Results"
        self.intermediate_folder = "Intermediate Results"
        self.final_results_folder = "Final Results"
        self.folder = folder
        self.debug_levels = {0: "Final", 1: "Intermediate", 2: "Debug"}
        self.debug_label = debug_level
        self._create_directories_()

    def _create_directories(self):
        create_dir_recursively(os.path.join(self.folder, self.intermediate_folder), False)
        create_dir_recursively(os.path.join(self.folder, self.final_results_folder), False)
        create_dir_recursively(os.path.join(self.folder, self.debug_folder), False)
        self.__create_intermediate_folders_()
        self._create_debug_folders_()
        self._create_final_folders_()

    def _create_intermediate_folders_(self):
        self.accuracies_file = os.path.join(self.folder, self.intermediate_folder, 'Model Accuracies.pickle')
        self.vulnerable_file = os.path.join(self.folder, self.intermediate_folder, 'Vulnerable Classes.pickle')
        self.model_result_file_path = os.path.join(self.folder, self.intermediate_folder, 'Model Results.csv')
        self.models_folder = os.path.join(self.folder, self.intermediate_folder, 'Models')

    def _create_debug_folders_(self):
        self.learning_log_file = os.path.join(self.folder, self.debug_folder, 'learning.log')
        self.plotting_log_file = os.path.join(self.folder, self.debug_folder, 'plotting.log')
        self.pvalue_cal_log_file = os.path.join(self.folder, self.debug_folder, 'p-value-calculation.log')

    def _create_final_folders_(self):
        self.plots_folder = os.path.join(self.folder, self.final_results_folder, 'Plots')
        self.importance_folder = os.path.join(self.plots_folder, 'Feature Importance')
        self.learning_curves_folder = os.path.join(self.plots_folder, 'Learning Curves')
        self.result_file_path = os.path.join(self.folder, self.final_results_folder, 'Final Results.csv')
        self.report_file = os.path.join(self.folder, self.final_results_folder, 'Report.txt')

    def remove_folders(self):
        if self.debug_label < 2:
            os.rmdir(self.debug_folder)
        if self.debug_label == 0:
            os.rmdir(self.intermediate_folder)

