<?php
/**
 * @file
 * An object that needed for basic module operations.
 */

/**
 * Class FppBundlesModule.
 */
final class FppBundlesModule extends \FppBundlesDatabase {
  const ENTITY_URL = 'admin/structure/fieldable-panels-panes';
  const ENTITY_TYPE = 'fieldable_panels_pane';
  const ENTITY_ACCESS = 'administer fieldable panels panes';

  /**
   * Instance of object for provide "singleton" pattern.
   *
   * @var self
   */
  private static $instance;

  /**
   * The variable is needed for use it as prefix for different targets.
   *
   * @var string
   */
  protected $moduleName;

  /**
   * Relative path to module from Drupal installation directory.
   *
   * @var string
   */
  protected $modulePath;

  /**
   * An associative array where key is name of the action and a value - URL.
   *
   * @var array
   */
  protected $menuPaths = array();

  /**
   * @var array
   *   An associative array where key is name of the action and a value - title.
   */
  protected $actionTitles = array();

  /**
   * An associative array that contains the Drupal menu routing items.
   *
   * @var array
   */
  protected $menuRoutes = array();

  /**
   * Creates general module information.
   *
   * @param string $module_root_dir
   *   Absolute or relative path to the directory with a module.
   */
  protected function __construct($module_root_dir) {
    $this->moduleName = basename($module_root_dir);
    $this->modulePath = str_replace(DRUPAL_ROOT . '/', '', $module_root_dir);

    parent::__construct($this->moduleName);

    foreach (array(
      'autocomplete category' => array(
        'type' => MENU_CALLBACK,
        'page callback' => TRUE,
        'page arguments' => array(5),
      ),
      'remove' => array(
        'title' => t('Remove the bundle'),
      ),
      'update' => array(
        'title' => t('Update the bundle'),
      ),
      'create' => array(
        'type' => MENU_LOCAL_ACTION,
        'title' => t('Create the bundle'),
      ),
    ) as $human_name => $data) {
      $relative_path = str_replace(' ', '/', $human_name);
      // All menu routes of the module relative to the next URL:
      // "admin/structure/fieldable-panels-panes/list/{module_route}".
      // That's necessary to be a part of the FPP module and complement it.
      $route_path = self::ENTITY_URL . '/' . $relative_path;
      // Prefix for the menu callback function, by default it will be the
      // next: "_fpp_bundles_".
      $callback_prefix = '_' . $this->moduleName . '_';
      // The suffix of the routing callback function by default is a
      // "manage_bundle". Due to this, default callback function name
      // is a "_fpp_bundles_manage_bundle".
      $callback_suffix = 'manage_bundle';

      // If we don't want to use the default route function, then we can
      // override it, defining the "page callback". Suffix of the callback
      // function will be generated automatically from relative route path.
      if (isset($data['page callback'])) {
        $callback_suffix = $this->createMachineName($human_name);
        unset($data['page callback']);
      }

      $this->menuRoutes[$route_path] = $data + array(
        'type' => MENU_VISIBLE_IN_BREADCRUMB,
        'page callback' => $callback_prefix . $callback_suffix,
        'page arguments' => array(3),
        'access arguments' => array(self::ENTITY_ACCESS),
      );

      $this->menuPaths[$human_name] = $route_path;

      if (isset($data['title'])) {
        $this->actionTitles[$human_name] = $data['title'];
      }
    }
  }

  /**
   * Property getter.
   */
  public function getModulePath() {
    return $this->modulePath;
  }

  /**
   * Property getter.
   */
  public function getModuleName() {
    return $this->moduleName;
  }

  /**
   * Property getter.
   */
  public function getMenuPaths() {
    return $this->menuPaths;
  }

  /**
   * Property getter.
   */
  public function getMenuRoutes() {
    return $this->menuRoutes;
  }

  /**
   * Property getter.
   */
  public function getActionTitles() {
    return $this->actionTitles;
  }

  /**
   * Get the page title for an action.
   *
   * @param string $action
   *   Action name.
   *
   * @return string
   *   Title for the specified action.
   */
  public function getActionTitle($action) {
    return isset($this->actionTitles[$action]) ? $this->actionTitles[$action] : '';
  }

  /**
   * Return entity manage URL.
   */
  public function getEntityManageUrl() {
    return self::ENTITY_URL . '/manage';
  }

  /**
   * Get instance of the object or property value if it specified.
   *
   * @param string $module_root_dir
   *   Absolute path to the module ".module" file.
   *
   * @return self
   *   Instance of the object.
   */
  public static function instance($module_root_dir) {
    if (!self::$instance) {
      self::$instance = new self($module_root_dir);
    }

    return self::$instance;
  }

  /**
   * Transform any string to human readable name.
   *
   * @param string $string
   *   Input string for transformations.
   *
   * @return string
   *   Generated name.
   */
  public static function createName($string) {
    return trim(preg_replace('/\s+/', ' ', preg_replace('/[^\w+]|\+|_/i', ' ', $string)));
  }

  /**
   * Transform a any string to machine name.
   *
   * @example
   *   Input: "__-_   An awe-some manager!#$%^&_+123=-ÆæØøäÄö'Ö\"@ (bundle)"
   *   Output: "an_awe_some_manager123_bundle"
   *
   * @param string $string
   *   Input string for transformations.
   *
   * @return string
   *   Generated name.
   */
  public static function createMachineName($string) {
    // Firstly, set to lowercase the input string. Secondly, leave in a
    // row only lowercase latin characters, numbers, and single spaces,
    // but only between the words. Thirdly, replace spaces to underscores.
    return trim(preg_replace('/\s+|-+/', '_', trim(preg_replace('/[^a-z\d\s-]/', '', strtolower($string)))), '_');
  }

  /**
   * Clear caches of the Drupal theme and entities.
   */
  public static function clearCaches() {
    entity_info_cache_clear();
    drupal_theme_rebuild();
  }
}
