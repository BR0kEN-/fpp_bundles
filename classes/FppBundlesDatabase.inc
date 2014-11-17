<?php
/**
 * @file
 * Object for working with stored panels in DB.
 */

/**
 * Class FppBundlesDatabase.
 */
class FppBundlesDatabase {
  const BUNDLE_CLASS_NAME = '\FppBundle';

  /**
   * Database schema for Drupal "hook_schema" usage.
   *
   * @var array
   */
  protected $schema = array();

  /**
   * Table fields names, exclude indexes.
   *
   * @var array
   */
  protected $fields = array();

  /**
   * An array where key is name the index and value - field with index.
   *
   * @var array
   */
  protected $indexes = array(
    'primary' => '',
    'unique' => '',
  );

  /**
   * An array with all bundles created from admin interface.
   *
   * @var array
   */
  private $allBundles = array();

  /**
   * DB table name.
   *
   * @var string
   */
  private $table = '';

  /**
   * Create instance to work with DB table.
   *
   * @param string $table_name
   *   The name of table in DB.
   */
  protected function __construct($table_name) {
    $this->table = $table_name;
    $not_null_varchar = array(
      'type' => 'varchar',
      'length' => 100,
      'not null' => TRUE,
    );

    $indexes = array(
      // Primary key - auto increment.
      'bid' => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'description' => 'Bundle ID',
      ),
      // Unique key - generated automatically.
      'machine' => $not_null_varchar + array(
        'description' => 'Machine name of the bundle',
      ),
    );

    $fields = array(
      // Input in UI. Required field on base of which
      // will be generated a machine name of the bundle.
      'name' => $not_null_varchar + array(
        'description' => 'Human name of the bundle',
      ),
      // Checkbox in UI. Needed for the indicated where entity
      // will be shown: in category or in general list.
      'level' => array(
        'type' => 'int',
        'not null' => TRUE,
        'description' => 'Indicates that the panel is "top level" or placed into category.',
      ),
      // Input in UI.
      'category' => $not_null_varchar + array(
        'description' => 'Optional field for store bundles in categories.',
      ),
    );

    $this->indexes = array_combine(array_keys($this->indexes), array_keys($indexes));
    $this->fields = array_keys($fields);

    // Create DB schema for the storage module data.
    $this->schema = array(
      $this->table => array(
        'fields' => $indexes + $fields,
        'primary key' => array($this->indexes['primary']),
        'unique keys' => array($this->indexes['unique'] => array($this->indexes['unique'])),
        'description' => 'Storage for Fieldable Panels Panes bundles',
      ),
    );
  }

  /**
   * Property getter.
   */
  public function getSchema() {
    return $this->schema;
  }

  /**
   * Get the bundle by one of the indexes in table.
   *
   * @param string $index_name
   *   An index name. Must be "primary" or "unique".
   * @param int|string $value
   *   An ID of the bundle.
   *
   * @return bool|array
   *   False - if nothing was found or an array with info about bundle.
   */
  public function getBundleByDbIndex($index_name, $value) {
    if (!isset($this->indexes[$index_name])) {
      return FALSE;
    }

    return $this->prepareBundles()
      ->condition($this->indexes[$index_name], $value, '=')
      ->execute()
      ->fetchObject(self::BUNDLE_CLASS_NAME);
  }

  /**
   * Get all bundles from the DB.
   *
   * @param int $per_page
   *   Number of items per page.
   *
   * @return array
   *   An array where each item is an instance of the \FppBundle;
   */
  public function getAllBundles($per_page = 0) {
    if (!$this->allBundles) {
      $query = $this->prepareBundles();

      if ($per_page && is_int($per_page)) {
        /* @var \PagerDefault $query */
        $query = $query->extend('PagerDefault');
        $query->limit($per_page);
      }

      $this->allBundles = $query->execute()->fetchAll(
        \PDO::FETCH_CLASS, self::BUNDLE_CLASS_NAME
      );

      /* @var \FppBundle $bundle */
      foreach ($this->allBundles as $i => $bundle) {
        $this->allBundles[$bundle->getMachineName()] = $bundle;
        unset($this->allBundles[$i]);
      }
    }

    return $this->allBundles;
  }

  /**
   * Get bundle by it machine name.
   *
   * @param string $machine_name
   *   Machine readable name of the bundle.
   *
   * @return \FppBundle|bool
   *   An instance ob the bundle object or false if it does not exist.
   */
  public function getBundleByName($machine_name) {
    $bundles = $this->getAllBundles();

    return isset($bundles[$machine_name]) ? $bundles[$machine_name] : FALSE;
  }

  /**
   * Get machine names of the all bundles.
   *
   * @return array
   *   An array where each item - an unique machine name of the bundle.
   */
  public function getAllBundleMachineNames() {
    return array_keys($this->getAllBundles());
  }

  /**
   * Get all field names from DB table where bundles was stored.
   *
   * @return array
   *   An array with fields machine names.
   */
  public function getAllSchemaFields() {
    $fields = $this->fields;
    $fields[] = $this->indexes['unique'];
    $fields[] = $this->indexes['primary'];

    return $fields;
  }

  /**
   * Select FPP bundles from DB.
   *
   * @return \SelectQuery
   *   Database object for select query.
   */
  public function prepareBundles() {
    return db_select($this->table)->fields($this->table, $this->getAllSchemaFields());
  }

  /**
   * Save values for the bundle and create it.
   *
   * @param array $bundle
   *   Bundle object.
   *
   * @return int
   *   Last insert ID.
   */
  public function createBundle(array $bundle) {
    return db_insert($this->table)->fields($bundle)->execute();
  }

  /**
   * Update existing bundle.
   *
   * @param array $bundle
   *   Bundle object.
   */
  public function updateBundle(array $bundle) {
    $bid = $bundle[$this->indexes['primary']];
    unset($bundle[$this->indexes['primary']]);

    db_update($this->table)
      ->fields($bundle)
      ->condition($this->indexes['primary'], $bid, '=')
      ->execute();
  }

  /**
   * Remove existing bundle.
   *
   * @param array $bundle
   *   Bundle object.
   */
  public function removeBundle(array $bundle) {
    db_delete($this->table)->condition($this->indexes['primary'], $bundle[$this->indexes['primary']], '=')->execute();
  }

  /**
   * Create an object representation of the bundle.
   *
   * @param array $data
   *   Keyed array with information about bundle.
   *   Required keys are: "name", "level", "machine", "category".
   *
   * @throws \FppBundlesException
   *   If property is absent but should be.
   *
   * @return array
   *   Bundle information.
   */
  public function prepareBundleData(array $data) {
    $save = array();

    foreach ($this->getAllSchemaFields() as $field) {
      if (isset($data[$field])) {
        $save[$field] = $data[$field];
      }
      elseif ($field !== $this->indexes['primary']) {
        throw \FppBundlesException::set('The property "!property" is not specified or does not exist, but it is necessary to create the bundle.', array(
          '!property' => $field,
        ));
      }
    }

    return $save;
  }
}
