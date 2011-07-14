<?php
/**
 * @file
 * Availability Calendars: admin settings form code
 * - define form
 * - validate
 * - submit
 *
 * @author Dan Karran (geodaniel) <dan at karran dot net>
 * @author Nicholas Alipaz (nicholas.alipaz)
 * @author Erwin Derksen (http://drupal.org/user/750928)
 */

module_load_include('inc', 'availability_calendars', 'availability_calendars');

/**
 * Retrieve form callback for the admin/config/availability-calendars/settings page.
 *
 * @return array The form
 */
function availability_calendars_admin_settings($form) {
  drupal_add_css(drupal_get_path('module', 'availability_calendars') . '/availability_calendars.admin.css', array('every_page' => FALSE));
  $form['#validate'][] = 'availability_calendars_admin_settings_validate';
  $form['#submit'][] = 'availability_calendars_admin_settings_submit';

  $settings = availability_calendars_get_settings();

  $form['availability_calendars_settings_content_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Availability calendar enabled content types'),
    '#description' => t('Availability calendars allows you to enable the calendar component for any content type. Choose the types on which you would like to associate availability calendar.'),
    '#options' => node_type_get_names(),
    '#default_value' => $settings->contenttypes,
  );
  $form['availability_calendars_settings_system_nodeview'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show automatically on node page'),
    '#default_value' => $settings->nodeview,
    '#description' => t("Check whether you want availability calendars to be displayed on the node page. If you don't check this, you will need to output it manually using the %func function.", array('%func' => "theme('availability_calendars_node')")),
  );
  $form['availability_calendars_settings_system_pernodeoverride'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow to override settings per node'),
    '#description' => t("If you check this setting, all settings in the '!block_name' block below will be overridden on a per node basis.", array('!block_name' => t('Global settings'))),
    '#default_value' => $settings->pernodeoverride,
  );
  $form['availability_calendars_settings_system'] = array(
    '#type' => 'fieldset',
    '#title' => t('Global settings'),
    '#description' => t("The following settings define the look & feel of the calendars on a system wide basis. If '!setting_name' is enabled, these can be overridden on a per node basis.", array('!setting_name' => t('Allow to override settings per node'))),
    '#tree' => TRUE,
  );
  $form['availability_calendars_settings_system']['showteaser'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show availability calendars within teasers'),
    '#default_value' => $settings->showteaser,
  );
  // Allow users to choose whether a key should be shown on the node on a per node basis.
  $form['availability_calendars_settings_system']['showkey'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show a key for the availability calendars'),
    '#default_value' => $settings->showkey,
  );
  // Allow users to choose whether the edit link should be shown or not.
  $form['availability_calendars_settings_system']['showeditlink'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show an edit link per month on the edit availability calendars tab'),
    '#description' => t('You will need to enable this option if you want to edit week notes or want to edit the statuses per day using dropdowns.'),
    '#default_value' => $settings->showeditlink,
  );
  // Allow users to choose whether the week notes should be shown or not.
  $form['availability_calendars_settings_system']['showweeknotes'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show a note before each week of the availability calendars'),
    '#default_value' => $settings->showweeknotes,
  );
  // Whether to use only the first letter for the day of the week or not.
  $form['availability_calendars_settings_system']['firstletter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use only the first letter from the day of the week'),
    '#default_value' => $settings->firstletter,
  );
  $form['availability_calendars_settings_system']['hideold'] = array(
    '#type' => 'checkbox',
    '#title' => t('Do not show availability state of dates in the past'),
    '#default_value' => $settings->hideold,
  );
  $form['availability_calendars_settings_system']['splitday'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow split day statuses'),
    '#default_value' => $settings->splitday,
  );
  $form['availability_calendars_settings_system']['startofweek'] = array(
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
  $form['availability_calendars_settings_system']['monthcount'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of months to display'),
    '#default_value' => $settings->monthcount,
    '#description' => t("The number of months to display to normal visitors."),
  );
  $form['availability_calendars_settings_system']['editormonthcount'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of months to display to editors'),
    '#default_value' => $settings->editormonthcount,
    '#description' => t("The number of months to display to visitors who are allowed to edit the calendar to be displayed. Setting this larger than the previous value allows editors to enter information into future calendars before it is made publicly available."),
  );

  // Add states.
  $form['states'] = array(
    '#type' => 'fieldset',
    '#title' => t('States'),
    '#tree' => TRUE,
    '#description' => t('<p>You can modify the availability states here.</p>
      <ul>
        <li>The label is what visitors will see in the legend and what editors will see when editing the calendar.</li>
        <li>The class must be unique and will be used for the css.</li>
        <li>Default defines the status to use for days that have a state not yet set.</li>
        <li>The weight defines the order in the legend and in the dropdown on the edit form.</li>
        <li>The "is available" checkbox defines whether this state should be treated as available in searches.</li>
        <li>Make a label empty to remove the row.</li>
        <li>If there are no more empty lines to add new states, save the form and you will be able to add another state.</li>
        <li><strong>Changes made to existing classes will not be updated in existing calendars!</strong> So do not change on live sites.</li>
        <li>If you define new classes, visit the styling page to define its colors.</li>
      </ul>
    '),
    '#attributes' => array('class' => array('state-list')),
  );

  $element = &$form['states'];
  $i = 0;
  foreach ($settings->states as $state) {
    availability_calendars_admin_settings_add_state($element, $i, $state, $settings);
    $i++;
  }

  // Show a minimum of 4 available states with at least one empty state.
  do {
    availability_calendars_admin_settings_add_state($element, $i, array('class' => '', 'label' => '', 'weight' => 0, 'is_available' => 0), $settings);
    $i++;
  } while ($i < 4);

  return system_settings_form($form);
}

/**
 * Helper function to add a state item to a form.
 * Only the first item gets labels, as the fields will be presented below each other.
 *
 * @param array $element
 *   The form element of which to add the state item.
 * @param int $i
 *   The state item count.
 * @param array $state
 *   Array containing a state record.
 * @param array $settings
 *   Array containing general (i.e. non per state) settings.
 */
function availability_calendars_admin_settings_add_state(&$element, $i, $state, $settings) {
  static $max_weight = 0;
  $element[$i]['label'] = array(
    '#type' => 'textfield',
    '#title' => $i == 0 ? t('Label') : '',
    '#default_value' => $state['label'],
    '#size' => 40,
    '#prefix' => '<div class="state-item">',
  );
  $element[$i]['class'] = array(
    '#type' => 'textfield',
    '#title' => $i == 0 ? t('Class') : '',
    '#default_value' => $state['class'],
    '#size' => 24,
  );
  // Bit difficult to add a set of related radio button's that do not appear together.
  $element[$i]['defaultstatus'] = array(
    '#type' => 'radio',
    '#tree' => FALSE,
    '#title' => $i == 0 ? t('Default') : '',
    '#title_display' => 'before',
    '#return_value' => $i,
    '#default_value' => !empty($state['class']) && $state['class'] == $settings->defaultstatus ? $i : 0,
    // Use the name attribute to group the radio buttons scattered over the items/rows.
    '#attributes' => array('name' => 'defaultstatus'),
  );
  $element[$i]['is_available'] = array(
    '#type' => 'checkbox',
    '#title' => $i == 0 ? t('Is available?') : '',
    '#title_display' => 'before',
    '#default_value' => $state['is_available'],
  );
  $element[$i]['weight'] = array(
    '#type' => 'select',
    '#title' => $i == 0 ? t('Weight') : '',
    '#default_value' => $state['weight'] > 0 ? $state['weight'] : ++$max_weight,
    '#options' => array_combine(range(1, 20, 1), range(1, 20, 1)),
    '#suffix' => '</div>',
  );
  if ($state['weight'] > $max_weight) {
    $max_weight = $state['weight'];
  }
}

/**
 * Form validate callback for the admin_settings form.
 * - At least one label should be filled.
 * - Entered classes should be a valid css class.
 * @param array $form
 * @param array $form_state
 */
function availability_calendars_admin_settings_validate($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  if ($op == t('Save configuration')) {
    $element = $form_state['values']['states'];
    $all_empty = TRUE;
    foreach ($element as $i => $state_fields) {
      if (!empty($state_fields['label'])) {
        $all_empty = FALSE;
      }
      if (!empty($state_fields['class']) && !availability_calendars_is_safe_id($state_fields['class'])) {
        form_set_error("states][$i][class", t('Class should be usable as a valid css class.'));
      }
    }

    if ($all_empty) {
      form_set_error('states][0][label', t('At least 1 state should be defined.'));
    }

    $default = $form_state['values']['defaultstatus'];
    if (empty($element[$default]['label'])) {
      form_set_error("states][$default][label", t('The default state may not be empty or deleted.'));
    }
  }
}

/**
 * Form submit callback for the admin_settings form.
 *
 * Processes the submitted form. The states are non-system settings, and are handled here.
 * Other values are handled by the default system settings form handling.
 * @param array $form
 * @param array $form_state
 */
function availability_calendars_admin_settings_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  if ($op == t('Save configuration')) {
    // Process the state settings here and do not pass them to the default submit handler for system settings forms.
    $element = $form_state['values']['states'];
    unset($form_state['values']['states']);

    // Extract the states.
    $states = array();
    foreach ($element as $i => $state_fields) {
      // Only add non-empty labels.
      if (!empty($state_fields['label'])) {
        if (empty($state_fields['class'])) {
          $state_fields['class'] = availability_calendars_string_to_safe_id($state_fields['label']);
        }
        $states[$state_fields['class']] = array(
          'class' => $state_fields['class'],
          'label' => $state_fields['label'],
          'weight' => $state_fields['weight'],
          'is_available' => $state_fields['is_available'],
        );
      }
    }

    $default = $form_state['values']['defaultstatus'];
    if (!empty($element[$default]['class'])) {
      $form_state['values']['availability_calendars_settings_system']['defaultstatus'] = $element[$default]['class'];
    }

    // Finally, save the states.
    availability_calendars_update_states($states);

    // Remove all per node settings if the setting changes from yes to no.
    if ($form_state['values']['availability_calendars_settings_system_pernodeoverride'] == 0) {
      $settings = availability_calendars_get_settings();
      if ($settings->pernodeoverride) {
        availability_calendars_delete_node_settings();
      }
    }

    // Clean-up the representation of supported content types before storing as a variable.
    $form_state['values']['availability_calendars_settings_content_types'] = array_keys(array_filter($form_state['values']['availability_calendars_settings_content_types']));
  }
}

/**
 * Converts a string to a valid html id/class attribute.
 *
 * http://www.w3.org/TR/html4/struct/global.html#h-7.5.2 specifies what makes a
 * valid id/class attribute in HTML. This function:
 *
 * - Ensure an ID starts with an alpha character by prefixing with 'cal'.
 * - Replaces any character except a-z, A-Z, numbers, and underscores with dashes.
 * - Converts entire string to lowercase.
 *
 * @param string $string
 * @return string the converted string
 */
function availability_calendars_string_to_safe_id($string) {
  return 'cal' . drupal_html_class($string);
}

/**
 * Checks whether the given string can be used as an html id or class.
 *
 * @param string $string
 * @return boolean
 */
function availability_calendars_is_safe_id($string) {
  return preg_match('/^[a-zA-Z][a-zA-Z0-9_-]*$/', $string) > 0;
}
