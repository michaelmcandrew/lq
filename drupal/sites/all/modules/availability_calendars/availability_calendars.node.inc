<?php
/**
 * @file
 * Availability Calendars: code to edit the calendar and per node settings.
 *
 * @author Dan Karran (geodaniel) <dan at karran dot net>
 * @author Nicholas Alipaz (nicholas.alipaz)
 * @author Erwin Derksen (http://drupal.org/user/750928)
 */

module_load_include('inc', 'availability_calendars', 'availability_calendars');


/**
 * Alters the form for node edit forms for supported content types.
 *
 * Called by our hook_form_BASE_FORM_ID_alter implementation. For parameters
 * @see http://api.drupal.org/api/drupal/modules--system--system.api.php/function/hook_form_BASE_FORM_ID_alter/7
 */
function availability_calendars_node_edit_form_alter(&$form, &$form_state, $form_id) {

  $node = node_load($form['nid']['#value']);
  $calendar_nid = $node->tnid && $node->tnid != $node->nid ? $node->tnid : $node->nid;
  $settings = availability_calendars_get_settings('node', $calendar_nid);

  $form['availability_calendars'] = array(
    '#type' => 'fieldset',
    '#title' => t('Availability calendar settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#tree' => TRUE,
  );
  $form['availability_calendars']['showteaser'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show availability calendars within teasers'),
    '#default_value' => $settings->showteaser,
  );
  // Allow users to choose whether a key should be shown on the node on a per node basis.
  $form['availability_calendars']['showkey'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show a key for the availability calendars'),
    '#default_value' => $settings->showkey,
  );
  // Allow users to choose whether the edit link should be shown or not.
  $form['availability_calendars']['showeditlink'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show an edit link per month on the edit availability calendar tab'),
    '#description' => t('You will need to enable this option if you want to edit week notes or want to edit the statuses per day using dropdowns.'),
    '#default_value' => $settings->showeditlink,
  );
  // Allow users to choose whether the week notes should be shown or not.
  $form['availability_calendars']['showweeknotes'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show a note before each week of the availability calendars'),
    '#default_value' => $settings->showweeknotes,
  );
  // Whether to use only the first letter for the day of the week or not.
  $form['availability_calendars']['firstletter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use only the first letter from the day of the week'),
    '#default_value' => $settings->firstletter,
  );
  $form['availability_calendars']['hideold'] = array(
    '#type' => 'checkbox',
    '#title' => t('Do not show availability state of dates in the past'),
    '#default_value' => $settings->hideold,
  );
  $form['availability_calendars']['splitday'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow split day statuses'),
    '#default_value' => $settings->splitday,
  );
  $form['availability_calendars']['startofweek'] = array(
    '#type' => 'select',
    '#title' => t('First day of week'),
    '#default_value' => $settings->startofweek,
    '#options' => array(
      6 => t('Monday'),
      5 => t('Tuesday'),
      4 => t('Wednesday'),
      3 => t('Thursday'),
      2 => t('Friday'),
      1 => t('Saturday'),
      0 => t('Sunday'),
    ),
  );
  $form['availability_calendars']['monthcount'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of months to display'),
    '#default_value' => $settings->monthcount,
  );
  $form['availability_calendars']['editormonthcount'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of months to display to editors'),
    '#default_value' => $settings->editormonthcount,
    '#description' => t("The number of months to display to visitors who are allowed to edit the calendar to be displayed. Setting this larger than the previous value allows editors to enter information into future calendars before it is made publicly available."),
  );
  $form['availability_calendars']['defaultstatus'] = array(
    '#type' => 'select',
    '#title' => t('Default status for each day'),
    '#default_value' => $settings->defaultstatus,
    '#options' => availability_calendars_options(),
  );
}

/**
 * Saves a set of settings associated with the node after inserting/updating it.
 *
 * @param object $node Node settings information to save into the database.
 */
function availability_calendars_save_node_settings($node) {
  variable_set('availability_calendars_settings_node_' . $node->nid, (array) $node->availability_calendars);
}

/**
 * Show the availability calendar (callback for path availability-calendars/%).
 *
 * @param object|int $node
 *   The node to show the calendar for (or a node id: deprecated format, remove)).
 * @param int|NULL $year
 *   The year to show the calendar for. If NULL, show from current month onwards.
 * @param int|NULL $month
 *   The month to show the calendar for. If NULL, show the whole year.
 * @param boolean $edit
 *   Whether to show the edit form for the given year and month?
 * @return string
 *   Themed calendar or edit calendar form.
 */
function availability_calendars_edit_calendar($node = NULL, $year = NULL, $month = NULL, $edit = FALSE) {
  if (is_numeric($node)) {
    $node = node_load($node);
  }
  if (is_object($node)) {
    drupal_set_title(t('Availability for !name', array('!name' => $node->title)));
    $calendar_node = $node->tnid && $node->tnid != $node->nid ? node_load($node->tnid) : $node;
    $settings = availability_calendars_get_settings('node', $calendar_node->nid);

    if (is_numeric($year) && is_numeric($month)) {
      $settings->monthcount = 1;
    }
    elseif (is_numeric($year)) {
      // Display availability calendar for a whole year.
      $month = 1;
      $settings->monthcount = 12;
    }
    else {
      // Display rolling availability calendar from this point onwards.
      $year = date('Y');
      $month = date('m');
    }

    if ($edit == 'edit') {
      // Display the edit form for the availability calendar on the node.
      drupal_set_title(t('Availability for !name in !date', array('!name' => $node->title, '!date' => format_date(mktime(12, 0, 0, $month, 1, $year), 'custom', 'F Y'))));
      return drupal_get_form('availability_calendars_node_edit_calendar_month_form', $node, $year, $month, $settings);
    }
    else {
      return array(
        '#theme' => 'availability_calendars_node',
        '#node' => $calendar_node,
        '#year' => $year,
        '#month' => $month,
        '#settings' => $settings,
      );
    }
  }
  else {
    print drupal_not_found();
  }
}

/**
 * Populates the node edit calendar month form.
 *
 * @return array
 */
function availability_calendars_node_edit_calendar_month_form($form, $form_state, $node, $year, $month, $settings) {
  $nid = $node->nid;
  $form['nid'] = array(
    '#type' => 'hidden',
    '#default_value' => $nid,
  );
  $form['year'] = array(
    '#type' => 'hidden',
    '#default_value' => $year,
  );
  $form['month'] = array(
    '#type' => 'hidden',
    '#default_value' => $month,
  );

  $month_meta = availability_calendars_month_meta($year, $month, $settings);

  // find all entries in database for this month (notes and availability) and pre-populate.
  $notes = availability_calendars_get_node_notes($nid, $year, $month);
  $states = availability_calendars_get_node_states($nid, $year, $month, $settings);

  $day = 1;
  for ($week = 1; $week <= $month_meta['weeksinmonth']; $week++) {
    $form['week-' . $week] = array(
      '#type' => 'fieldset',
      '#title' => t('Week @week', array('@week' => $week)),
    );
    if ($settings->showweeknotes) {
      $form['week-' . $week]['note-' . $week] = array(
        '#type' => 'textfield',
        '#title' => t('Note'),
        '#default_value' => $notes[$week],
        '#description' => t('This will be displayed beside the week in the calendar. It could include, for example, a weekly price.'),
      );
    }
    if ($week == 1) {
      $daysinweekremaining = 7 - $month_meta['firstday'];
    }
    else {
      $daysinweekremaining = 7;
    }
    while ($daysinweekremaining > 0 && $day <= $month_meta['daysinmonth']) {
      $date = mktime(12, 0, 0, $month, $day, $year);
      $form['week-' . $week]['day-' . $day] = array(
        '#type' => 'select',
        '#title' => format_date($date, 'custom', 'l j F'),
        '#options' => availability_calendars_options(),
        '#default_value' => $states[date(AC_ISODATE, $date)],
      );
      $day++;
      $daysinweekremaining--;
    }
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
  );

  return $form;
}

/**
 * Callback function for submitting a node edit form.
 *
 * @param array $form
 * @param array $form_state
 */
function availability_calendars_node_edit_calendar_month_form_submit($form, &$form_state) {
  $nid = $form_state['values']['nid'];
  $year = $form_state['values']['year'];
  $month = $form_state['values']['month'];

  // extract weekly notes from the form
  $notes = array();
  $week = 1;
  $nomoreweeks = FALSE;
  while (!$nomoreweeks) {
    if (isset($form_state['values']['note-' . $week])) {
      $notes[$week] = $form_state['values']['note-' . $week];
      $week++;
    }
    else {
      $nomoreweeks = TRUE;
    }
  }

  // Extract states from the form.
  $states = array();
  $day = 1;
  $nomoredays = FALSE;
  while (!$nomoredays) {
    if (isset($form_state['values']['day-' . $day])) {
      $states[$day] = $form_state['values']['day-' . $day];
      $day++;
    }
    else {
      $nomoredays = TRUE;
    }
  }

  // Update.
  if (!empty($notes)) {
    availability_calendars_update_node_notes($nid, $year, $month, $notes);
  }
  availability_calendars_update_node_states($nid, $year, $month, $states);

  // Clear the page and block caches (#747992).
  // These lines are copied from node_form_submit() and node_save().
  // Note: cleaning the cache ourselves, means that the fields 'changed' and 'timestamp' won't be updated.
  // If that is what we want, call node_save(). However, note that in D7, cache_clear_all() gets called by
  // node_form_submit(), not by node_save() (as in D6).
  entity_get_controller('node')->resetCache(array($nid));
  cache_clear_all();

  drupal_set_message(t('Availability information saved.'));
}

/**
 * Callback to retrieve the form for the node/%node/availability-calendars page.
 *
 * @return array
 *   The form.
 */
function availability_calendars_node_edit_calendar_form($form, $form_state, $node) {
  drupal_set_title(t('<em>Edit availability calendar</em> @title', array('@title' => $node->title)), PASS_THROUGH);
  $formHandler = AvailabilityCalendarsNodeEditCalendarFormHandler::instance($node);
  return $formHandler->build($form);
}

/**
 * @see AvailabilityCalendarsNodeEditCalendarFormHandler::processRadios()
 */
function availability_calendars_node_edit_calendar_form_process_radios($element) {
  return AvailabilityCalendarsNodeEditCalendarFormHandler::instance()->processRadios($element);
}

/**
 * Callback to validate the form for the styles form.
 */
function availability_calendars_node_edit_calendar_form_validate($form, &$form_state) {
  return AvailabilityCalendarsNodeEditCalendarFormHandler::instance()->validate($form, $form_state);
}

/**
 * Callback to process form submission for the styles form.
 */
function availability_calendars_node_edit_calendar_form_submit($form, &$form_state) {
  return AvailabilityCalendarsNodeEditCalendarFormHandler::instance()->submit($form, $form_state);
}

class AvailabilityCalendarsNodeEditCalendarFormHandler {
  private static $instance = NULL;
  /** @var $node object */
  protected $node;
  /** @var $settings object */
  protected $settings;
  /** @var $states array */
  protected $states;
  /** @var $states array */
  protected $commands;

  /**
   * @param object $node
   * @return AvailabilityCalendarsNodeEditCalendarFormHandler
   */
  public static function instance($node = NULL) {
    if (self::$instance === NULL) {
      self::$instance = new AvailabilityCalendarsNodeEditCalendarFormHandler($node);
    }
    return self::$instance;
  }

  protected function __construct($node) {
    $this->node = $node->tnid && $node->tnid != $node->nid ? node_load($node->tnid) : $node;
    $this->settings = availability_calendars_get_settings('node', $this->node->nid);
    // Show the editor month count.
    $this->settings->monthcount = $this->settings->editormonthcount;
    // Do not show the key. We show our own key here with radio buttons and no special states.
    $this->settings->showkey = false;
  }

  /**
   * Builds the node/%node/availability-calendars page form.
   *
   * @param array $form
   * @return array
   *   The modified form.
   */
  public function build($form) {
    // Add js or the edit functionality.
    availability_calendars_add_js('edit', NULL, $this->settings);

    // Add form fields.
    $year = date('Y');
    $month = date('m');
    $form['availability_calendar'] = array(
      '#type' => 'markup',
      '#theme' => 'availability_calendars_node',
      '#markup' => '',
      '#prefix' => '<div class="description">'
        . t('To update the calendar: select a state (below the calendar) and click on a begin and end date. Repeat. When finished, click on <em>@button</em>.', array('@button' => t('Update')))
        . '</div>',
      '#node' => $this->node,
      '#year' => $year,
      '#month' => $month,
      '#settings' => $this->settings,
    );
    $options = array();
    foreach ($this->settings->states as $class => $state) {
      $options[$class] = t($state['label']);
    }
    $form['availability_states'] = array(
      '#type' => 'radios',
      '#title' => t('States'),
      '#default_value' => $this->settings->defaultstatus,
      '#options' => $options,
    );
    $form['availability_changes'] = array(
      '#type' => 'hidden',
      '#title' => t('Changes in availability'),
      '#default_value' => '',
      '#attributes' => array('id' => 'edit-availability-changes'),
    );

    // Add form button.
    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Update'),
    );
    return $form;
  }

  /**
   * Callback to validate the form for the styles form.
   */
  public function validate($form, &$form_state) {
    $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
    if ($op == t('Update')) {

      $this->commands = array();
      $changes = $form_state['values']['availability_changes'];
      // Silently ignore empty updates.
      if (!empty($changes)) {
        $changes = explode("\n", $changes);
        foreach ($changes as $change) {
          // Ignore empty lines (e.g. the last line).
          if (!empty($change)) {
            $command = $this->validateCommand($change);
            if ($command === FALSE) {
              form_set_error('availability_changes', t('The requested calendar changes contain an invalid request.'));
              break;
            }
            $this->commands[] = $command;
          }
        }
      }
    }
  }

  /**
   * Validates a single command line.
   *
   * @param string $change
   *   Command line: state: <string> from: yyyy-mm-dd[(pm)] to: yyyy-mm-dd[(am)]
   * @return boolean|array
   *   An array with key 'from', 'to' and 'state' representing the parsed and
   *   validated command line or FALSE on validation errors.
   */
  public function validateCommand($change) {
    // Basic syntax checking.
    $parts = explode(' ', trim($change));
    if (count($parts) !== 6
     || $parts[0] !== 'state:' || !array_key_exists($parts[1], $this->settings->states)
     || $parts[2] !== 'from:' || preg_match('/^[1-2][0-9]{3}-[0-1][0-9]-[0-3][0-9](\(pm\))?$/', $parts[3]) !== 1
     || $parts[4] !== 'to:' || preg_match('/^[1-2][0-9]{3}-[0-1][0-9]-[0-3][0-9](\(am\))?$/', $parts[5]) !== 1) {
      return FALSE;
    }
    // Check dates: valid date, in between ranges of the calendar, no am/pm when no split days
    // allowed, and from <= to (including the pm/am modifier: this prevents the "same day from
    // pm to am" situation.
    $year = date('Y');
    $month = date('m');
    if (!checkdate(substr($parts[3], 5, 2), substr($parts[3], 8, 2), substr($parts[3], 0, 4))
     || $parts[3] < date(AC_ISODATE, mktime(0, 0, 0, $month , 1, $year))) {
      return FALSE;
    }
    if (!checkdate(substr($parts[5], 5, 2), substr($parts[5], 8, 2), substr($parts[5], 0, 4))
     || $parts[5] > date(AC_ISODATE, mktime(0, 0, 0, $month + $this->settings->monthcount + 1, 0, $year))) {
      return FALSE;
    }
    if ($this->settings->splitday == '0' && (strlen($parts[3]) > 10 || strlen($parts[5]) > 10)) {
      return FALSE;
    }
    if ($parts[3] > $parts[5]) {
      return FALSE;
    }
    return array('from' => $parts[3], 'to' => $parts[5], 'state' => $parts[1]);
  }

  /**
   * Callback to process form submission for the styles form.
   */
  public function submit($form, &$form_state) {
    $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
    if ($op == t('Update')) {
      foreach ($this->commands as $command) {
        availability_calendars_update_node_states_range($this->node->nid, $command['from'], $command['to'], $command['state']);
      }

      // Redirect to node view (as is also done on the node edit form).
      $form_state['redirect'] = 'node/' . $this->node->nid;

      // Clear the page and block caches (don't regress #747992).
      entity_get_controller('node')->resetCache(array($this->node->nid));
      cache_clear_all();

      drupal_set_message(t('Availability information saved.'));
    }
  }
}
