<?php
/**
 * @file
 * Implements the Features API for export bundles as features.
 */

/**
 * Implements hook_features_export_options().
 */
function fpp_bundles_features_export_options() {
  $options = array();

  /* @var \FppBundle $bundle */
  foreach (fpp_bundles_db()->getAllBundles() as $machine_name => $bundle) {
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
    $bundle_fields = db_select('field_config_instance', 'fci')
      ->fields('fci', array('field_name'))
      ->condition('bundle', $bundle_machine_name, '=')
      ->execute()
      ->fetchCol();

    $export['features'][$module_name][$bundle_machine_name] = $bundle_machine_name;

    foreach ($bundle_fields as $field_name) {
      $field = features_field_base_load($field_name);

      if ($field) {
        $export['dependencies'][$field['module']] = $field['module'];
        $export['features']['field_base'][$field_name] = $field_name;
      }
    }
  }

  return array();
}

/**
 * Implements hook_features_export_render().
 */
function fpp_bundles_features_export_render($feature_name, $data, $export = NULL) {
  $code = array();

  foreach ($data as $bundle_machine_name) {
    $bundle = fpp_bundles_db()->getBundleByMachineName($bundle_machine_name);

    if ($bundle) {
      $code += $bundle->export();
    }
  }

  return array(
    FPP_BUNDLES_MACHINE_NAME => '  return ' . features_var_export($code, '  ') . ';',
  );
}

/**
 * Implements hook_features_rebuild().
 */
function fpp_bundles_features_rebuild($feature_name) {
  $db = fpp_bundles_db();

  foreach (module_invoke($feature_name, FPP_BUNDLES_MACHINE_NAME) as $machine_name => $values) {
    if (!$db->getBundleByMachineName($machine_name)) {
      $db->insertBundle($values);
    }
  }

  _fpp_bundles_clear_caches();
}

/**
 * Implements hook_features_revert().
 */
function fpp_bundles_features_revert($feature_name) {
  fpp_bundles_features_rebuild($feature_name);
}
