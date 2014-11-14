<?php
/**
 * @file
 * Implements the Features API for export bundles as features.
 */

/**
 * Implements hook_features_export_options().
 */
function fpp_bundles_features_export_options() {
  $bundles = fpp_bundles()->getAllBundles();
  $options = array();

  /* @var \FppBundle $bundle */
  foreach ($bundles as $machine_name => $bundle) {
    $options[$machine_name] = $bundle->getHumanName();
  }

  return $options;
}

/**
 * Implements hook_features_export().
 */
function fpp_bundles_features_export($data, &$export, $feature_name, $module_name) {
  $export['dependencies'][$module_name] = $module_name;

  foreach ($data as $bundle_machine_name) {
    $export['features'][$module_name][$bundle_machine_name] = $bundle_machine_name;
  }

  return array();
}

/**
 * Implements hook_features_export_render().
 */
function fpp_bundles_features_export_render($feature_name, $data, $export = NULL) {
  $module = fpp_bundles();
  $code = array();

  foreach ($data as $bundle_machine_name) {
    $bundle = $module->getBundleByName($bundle_machine_name);

    if ($bundle) {
      $code += $bundle->export();
    }
  }

  return array(
    $module->getModuleName() => '  return ' . features_var_export($code, '  ') . ';',
  );
}

/**
 * Implements hook_features_rebuild().
 */
function fpp_bundles_features_rebuild($feature_name) {
  $module = fpp_bundles();

  foreach (module_invoke($feature_name, $module->getModuleName()) as $machine_name => $values) {
    if (!$module->getBundleByName($machine_name)) {
      $module->createBundle($values);
    }
  }

  \FppBundlesModule::clearCaches();
}

/**
 * Implements hook_features_revert().
 */
function fpp_bundles_features_revert($feature_name) {
  fpp_bundles_features_rebuild($feature_name);
}
