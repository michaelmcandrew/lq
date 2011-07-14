<?php
/**
 * @file
 * General helper methods for Availability Calendars, like database access and settings.
 *
 * @author Dan Karran (geodaniel) <dan at karran dot net>
 * @author Nicholas Alipaz (nicholas.alipaz)
 * @author Erwin Derksen (http://drupal.org/user/750928)
 */

define('AC_ISODATE', 'Y-m-d');

/**
 * Utility function to create an array of meta data for the month.
 *
 * @param int $year
 * @param int $month
 * @param object $settings
 * @return array
 */
function availability_calendars_month_meta($year, $month, $settings) {
  $month_meta['daysinmonth'] = date("t", mktime(0, 0, 0, $month, 1, $year));
  $month_meta['firstday'] = date("w", mktime(0, 0, 0, $month, 1, $year)) + $settings->startofweek;
  $temp_days = $month_meta['firstday'] + $month_meta['daysinmonth']; // padding
  $month_meta['weeksinmonth'] = ceil($temp_days / 7);

  // Stop empty weeks occuring at start of month
  if ($month_meta['firstday'] > 6) {
    $month_meta['firstday'] = $month_meta['firstday'] - 7;
    $month_meta['weeksinmonth'] --;
  }
  return $month_meta;
}

/**
 * availability_calendars status options.
 * Note: we return unescaped labels as they might be used as options in a select where they get escaped again.
 *
 * @return array
 *   Array with the classes as the keys and the translated but unescaped labels as values.
 */
function availability_calendars_options() {
  static $ret = NULL;
  if ($ret === NULL) {
    $ret = array();
    $settings = availability_calendars_get_settings();
    $states = $settings->states;
    foreach ($states as $class => $state) {
      $ret[$class] = $state['label'];
    }
    if ($settings->splitday === 1) {
      foreach ($states as $class => $state) {
        $sub = $states;
        unset($sub[$class]);
        foreach ($sub as $subclass => $substate) {
          $ret["$class-am $subclass-pm"] = t('!a (am)/!b (pm)', array('!a' => $state['label'], '!b' => $substate['label']));
        }
      }
    }
  }
  return $ret;
}

/**
 * Utility function to get settings related to nodes or administration.
 *
 * @param string $scope
 *   Type of settings to get: node or system.
 * @param int|NULL $arg
 *   nid.
 * @return object
 *   An object with all settings for the given scope.
 */
function availability_calendars_get_settings($scope = NULL, $arg = NULL) {
  // Default settings.
  $states = availability_calendars_get_states();
  $first_state = reset($states);
  $settings = array(
    'states' => $states,
    'startofweek' => 1,
    'showteaser' => 1,
    'showkey' => 1,
    'showeditlink' => 1,
    'showweeknotes' => 1,
    'firstletter' => 0,
    'hideold' => 0,
    'defaultstatus' => $first_state['class'],
    'monthcount' => 12,
    'editormonthcount' => 18,
    'splitday' => 0,
    'nodeview' => variable_get('availability_calendars_settings_system_nodeview', 1),
    'pernodeoverride' => variable_get('availability_calendars_settings_system_pernodeoverride', 0),
  );

  // Override and extend with system wide settings.
  $settings = array_merge($settings, variable_get('availability_calendars_settings_system', array()));
  $settings['contenttypes'] = variable_get('availability_calendars_settings_content_types', array());

  // Override with node specific settings if we are in an existing node scope and allow to override per node.
  if ($scope === 'node') {
    $settings['nid'] = $arg;
    if ($settings['pernodeoverride'] && $arg !== NULL) {
      $settings = array_merge($settings, variable_get('availability_calendars_settings_node_' . $arg, array()));
    }
  }

  return (object) $settings;
}

/**
 * Returns a javascript settings array for the @see AvailabilityCalendars
 * javascript class.
 *
 * @param object $settings availability calendars settings object
 * @param string $selectable all|available|not-available|none
 * @return array Array with javascript settings that should be passed to
 *    @see drupal_add_js()
 */
function availability_calendars_get_js_settings_inc($settings, $selectable) {
  return array(
    'states' => $settings->states,
    'splitDay' => $settings->splitday ? TRUE : FALSE,
    'selectable' => $selectable,
  );
}

function availability_calendars_add_js_inc($mode, $js_settings, $settings) {
  static $api_added = FALSE;
  static $viewport_count = 0;

  if (!$api_added) {
    // Add the base client side API if not already done so
    drupal_add_js(drupal_get_path('module', 'availability_calendars') .'/availability_calendars.js', array('type' => 'file'));
    $api_added = TRUE;
  }

  if (stripos($mode, 'edit') !== FALSE) {
    drupal_add_js(drupal_get_path('module', 'availability_calendars') .'/availability_calendars.edit.js', array('type' => 'file'));
    drupal_add_js(array(
        'availabilityCalendars' => array(
          'edit' => array(
            'nid' => $settings->nid,
            'settings' => availability_calendars_get_js_settings($settings, 'all'),
          ),
        ),
      ), array('type' => 'setting'));
  }

  if (stripos($mode, 'viewport') !== FALSE) {
    drupal_add_js(drupal_get_path('module', 'availability_calendars') .'/availability_calendars.view.js', array('type' => 'file'));
    drupal_add_js(array(
        'availabilityCalendars' => array(
          "viewport$viewport_count" => array(
            'nid' => $settings->nid,
            'settings' => availability_calendars_get_js_settings($settings, 'available'),
            'viewport' => $js_settings,
          ),
        ),
      ), array('type' => 'setting'));
    drupal_add_js("Drupal.behaviors.availabilityCalendarsViewport$viewport_count = {
        attach: function(context, settings) {
          Drupal.availabilityCalendars.viewport$viewport_count = new Drupal.availabilityCalendars.Viewport(
            Drupal.availabilityCalendars.get(settings.availabilityCalendars.viewport$viewport_count.nid,
              settings.availabilityCalendars.viewport$viewport_count.settings),
            settings.availabilityCalendars.viewport$viewport_count.viewport.settings,
            settings.availabilityCalendars.viewport$viewport_count.viewport.backwardSelector,
            settings.availabilityCalendars.viewport$viewport_count.viewport.forwardSelector
          );
        }
      };", array('type' => 'inline', 'scope' => 'footer'));
    $viewport_count++;
  }
}


/*
 * DATABASE ACCESS FUNCTIONS
 * -------------------------
 */

/**
 * Implementation of availability_calendars_get_states (in .module)
 *
 * Returns an array of records of all states.
 * Optionally filtered by the is_available flag.
 *
 * @param boolean|NULL $is_available
 *   Filter on is_available state (boolean) or do not filter at all (NULL (default)).
 * @return array
 *   Array of records keyed by the class.
 */
function availability_calendars_get_states_inc($is_available) {
  static $states = NULL;
  if ($states === NULL) {
    $states = db_select('availability_calendars_states')
      ->fields('availability_calendars_states')
      ->orderBy('weight')
      ->execute()
      ->fetchAllAssoc('class', PDO::FETCH_ASSOC);
    array_walk($states, 'availability_calendars_convert_bool');
  }
  if ($is_available === NULL) {
    $result = $states;
  }
  else {
    // Filter sttes by is_available flag
    $result = array();
    foreach ($states as $class => $state) {
      if ($state['is_available'] == $is_available) {
        $result[$class] = $state;
      }
    }
  }

  return $result;
}

function availability_calendars_convert_bool(&$value) {
  $value['is_available'] = $value['is_available'] == 1;
}

/**
 * Updates the set of states.
 *
 * @param array $states
 *   Array with the new state records (class, label and weight values)
 */
function availability_calendars_update_states($states) {
  $existing_States = availability_calendars_get_states();
  if ($states != $existing_States) {
    // update states: delete all existing, insert all new states
    db_delete('availability_calendars_states')
      ->execute();
    foreach ($states as $state) {
      db_insert('availability_calendars_states')
        ->fields($state)
        ->execute();
    }
  }
}

/**
 * Returns the notes for the calendar for the given node in the given month.
 * The returned array will be completely filled, so no checking is necessary.
 *
 * @param int $nid
 * @param int $year
 * @param int $month
 * @return array
 *   Array with 6 week entries of week number (int) => note (string) (possibly empty string)
 */
function availability_calendars_get_node_notes($nid, $year, $month) {
  $notes = db_select('availability_calendars_week')
    ->fields('availability_calendars_week', array('week', 'note'))
    ->condition('nid', $nid)
    ->condition('year', $year)
    ->condition('month', $month)
    ->execute()
    ->fetchAllKeyed();
  // Complete the array with defaults
  $notes += array_fill(1, 6, "");
  return $notes;
}

/**
 * Updates the calendar notes for the given node in the given month.
 *
 * @param int $nid
 * @param int $year
 * @param int $month
 * @param array $notes
 *   The (possibly empty) notes keyed by week number.
 */
function availability_calendars_update_node_notes($nid, $year, $month, $notes) {
  // Delete all current notes.
  db_delete('availability_calendars_week')
      ->condition('nid', $nid)
      ->condition('year', $year)
      ->condition('month', $month)
      ->execute();
  // Insert new notes.
  $query = db_insert('availability_calendars_week');
  $values = array('nid' => $nid,'year' => $year,'month' => $month);
  $has_records = FALSE;
  foreach ($notes as $values['week'] => $values['note']) {
    if (!empty($values['note'])) {
      if (!$has_records) {
        $query->fields($values);
        $has_records = TRUE;
      }
      else {
        $query->values($values);
      }
    }
  }
  if ($has_records) {
    $query->execute();
  }
}

/**
 * Returns the states for the calendar for the given node in the given month.
 * The returned array will be completely filled, so no checking is necessary.
 *
 * @param int $nid
 * @param int $year
 * @param int $month
 * @param object $settings
 *   Settings (containing among others the default status)
 * @return array
 *   Array with 28 to 31 day states (string) keyed by the day of the month number (int).
 */
function availability_calendars_get_node_states($nid, $year, $month, $settings) {
  $start_date = date(AC_ISODATE, mktime(0, 0, 0, $month, 1, $year));
  $end_date = date(AC_ISODATE, mktime(0, 0, 0, $month + 1, 0, $year)); // Works
  $number_of_days = (int) substr($end_date, 8);

  // Create an array for all days of the month with the default status.
  $states = array();
  for($day = 1; $day <= $number_of_days; $day++) {
    $states[date(AC_ISODATE, mktime(0, 0, 0, $month, $day, $year))] = $settings->defaultstatus;
  }
  // Get the states from the database.
  $states_db = db_select('availability_calendars_day')
    ->fields('availability_calendars_day', array('date', 'status'))
    ->condition('nid', $nid)
    ->condition('date', array($start_date, $end_date), 'BETWEEN')
    ->execute()
    ->fetchAllKeyed();

  // Merge the 2 arrays.
  $states = array_merge($states, $states_db);
  return $states;
}

/**
 * Update the states for the calendar for the given node in the given month.
 *
 * @param int $nid
 * @param int $year
 * @param int $month
 * $param array $states
 *   Array with 28 to 31 day states (string) keyed by the day of the month number (int).
 */
function availability_calendars_update_node_states($nid, $year, $month, $states) {
  $start_date = date(AC_ISODATE, mktime(0, 0, 0, $month, 1, $year));
  $end_date = date(AC_ISODATE, mktime(0, 0, 0, $month + 1, 0, $year));
  // Delete all current states.
  db_delete('availability_calendars_day')
      ->condition('nid', $nid)
      ->condition('date', array($start_date, $end_date), 'BETWEEN')
      ->execute();
  // Insert new states.
  $query = db_insert('availability_calendars_day');
  $values = array('nid' => $nid);
  $has_records = FALSE;
  foreach ($states as $day => $values['status']) {
    $values['date'] = date(AC_ISODATE, mktime(0, 0, 0, $month, $day, $year));
    if (!$has_records) {
      $query->fields($values);
      $has_records = TRUE;
    }
    else {
      $query->values($values);
    }
  }
  if ($has_records) {
    $query->execute();
  }
}

/**
 * Returns the existing states for the calendar for the given node and date range.
 * The from and to dates are inclusive.
 *
 * @param int $nid
 * @param DateTime $from
 * @param DateTime $to
 * @return array
 *   Array with existing states within the given date range indexed by date.
 *   Missing dates should get the default status.
 */
function availability_calendars_get_node_states_range($nid, $from, $to) {
  // Get the states from the database.
  $states = db_select('availability_calendars_day')
    ->fields('availability_calendars_day', array('date', 'status'))
    ->condition('nid', $nid)
    ->condition('date', array($from->format(AC_ISODATE), $to->format(AC_ISODATE)), 'BETWEEN')
    ->execute()
    ->fetchAllKeyed();
  return $states;
}

/**
 * Updates/inserts the states for the calendar for the given node and date range.
 *
 * Note that $from and $to must be ordered and that if the date parts of $from and $to are equal,
 * no pm and am are allowed. Unexpected results will occur when not obeyed.
 *
 * @param int $nid
 * @param string $from
 *   Format: yyyy-mm-dd or yyyy-mm-dd(pm)
 * @param string $to
 *   Format: yyyy-mm-dd or yyyy-mm-dd(am)
 * $param string $state
 */
function availability_calendars_update_node_states_range($nid, $from, $to, $state) {
  $from_pm = drupal_substr($from, 10);
  /** @var $from DateTime */
  $from = new DateTime(drupal_substr($from, 0, 10));
  $to_am = drupal_substr($to, 10);
  /** @var $to DateTime */
  $to = new DateTime(drupal_substr($to, 0, 10));

  // Start building the insert query as specific from and to may already lead to insert values.
  $values = array('nid' => $nid, 'date' => null, 'status' => $state);
  $insert = db_insert('availability_calendars_day')
    ->fields(array_keys($values));

  // Get existing dates to be able to differentiate between update and insert.
  $existing_states = availability_calendars_get_node_states_range($nid, $from, $to);

  // Handle $from and $to dates specially when a split day range is passed in.
  if (!empty($from_pm)) {
    $values['date'] = $from->format(AC_ISODATE);
    if (array_key_exists($values['date'], $existing_states)) {
      // Update PM state, leave AM state, i.e. AM state becomes current whole day state or current AM state.
      $current_state = $existing_states[$values['date']];
      $matches = array();
      if (preg_match('/([^ ]+)-am/', $current_state, $matches) > 0) {
        $current_state = $matches[1];
      }
      $values['status'] = $current_state . '-am ' . $state . '-pm';
      db_update('availability_calendars_day')
        ->fields(array('status' => $values['status']))
        ->condition('nid', $nid, '=')
        ->condition('date', $values['date'], '=')
        ->execute();
    }
    else {
      // Insert default AM status and PM state.
      $settings = availability_calendars_get_settings('node', $nid);
      $values['status'] = $settings->defaultstatus . '-am ' . $state . '-pm';
      $insert->values($values);
    }
    $from->modify('+1 day');
  }
  if (!empty($to_am)) {
    $values['date'] = $to->format(AC_ISODATE);
    if (array_key_exists($values['date'], $existing_states)) {
      // Update AM state, leave PM state, i.e. PM state becomes current whole day state or current PM state.
      $current_state = $existing_states[$values['date']];
      $matches = array();
      if (preg_match('/([^ ]+)-pm/', $current_state, $matches) > 0) {
        $current_state = $matches[1];
      }
      $values['status'] = $state . '-am ' . $current_state . '-pm';
      db_update('availability_calendars_day')
        ->fields(array('status' => $values['status']))
        ->condition('nid', $nid, '=')
        ->condition('date', $values['date'], '=')
        ->execute();
    }
    else {
      // Insert AM state and default PM status.
      $settings = availability_calendars_get_settings('node', $nid);
      $values['status'] = $state . '-am ' . $settings->defaultstatus . '-pm';
      $insert->values($values);
    }
    $to->modify('-1 day');
  }

  // Only continue when handling split from and to dates did not cover the whole range.
  if ($from <= $to) {
    // Update the already existing dates.
    db_update('availability_calendars_day')
      ->fields(array('status' => $state))
      ->condition('nid', $nid, '=')
      ->condition('date', array($from->format(AC_ISODATE), $to->format(AC_ISODATE)), 'BETWEEN')
      ->execute();

    // Insert the non-existing dates.
    $values['status'] = $state;
    for ($day = $from; $day <= $to; $day->modify('+1 day')) {
      $values['date'] = $day->format(AC_ISODATE);
      if (!array_key_exists($values['date'], $existing_states)) {
        $insert->values($values);
      }
    }
  }
  $insert->execute();
}

/**
 * Deletes all calendar information for the given node.
 * - notes
 * - day states
 * - settings
 *
 * @param int $nid
 */
function availability_calendars_delete_node($nid) {
  $nid = (int) $nid;
  db_delete('availability_calendars_week')
    ->condition('nid', $nid)
    ->execute();
  db_delete('availability_calendars_day')
    ->condition('nid', $nid)
    ->execute();
  availability_calendars_delete_node_settings($nid);
}

/**
 * Removes the per node settings for one or all nodes.
 *
 * param int|NULL $nid
 *   Node id
 */
function availability_calendars_delete_node_settings($nid = NULL) {
  if ($nid === NULL) {
    // Remove per node settings for all nodes.
    db_delete('variable')
      ->condition('name', "availability_calendars_settings_node__%", 'LIKE')
      ->execute();
  }
  else {
    db_delete('variable')
      ->condition('name', "availability_calendars_settings_node_$nid", '=')
      ->execute();
  }
  cache_clear_all('variables', 'cache');
}
