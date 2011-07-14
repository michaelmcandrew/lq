<?php
/**
 * @file
 * Availability Calendars: theming code.
 * - Theme calendar
 * - Theme 1 month of a calendar
 * - Theme key to the calendar
 *
 * @author Dan Karran (geodaniel) <dan at karran dot net>
 * @author Nicholas Alipaz (nicholas.alipaz)
 * @author Erwin Derksen (http://drupal.org/user/750928)
 */

module_load_include('inc', 'availability_calendars', 'availability_calendars');

/**
 * Actual inplementation of D7 hook_node_view.
 * @param object $node
 * @param string $view_mode
 *   'full' or 'teaser'.
 */
function availability_calendars_page_node_view($node, $view_mode) {
  // If this is a translated node, use calendar of original node.
  $calendar_node = $node->tnid && $node->tnid != $node->nid ? node_load($node->tnid) : $node;
  $settings = availability_calendars_get_settings('node', $calendar_node->nid);
  if (($settings->nodeview === 1 && ($view_mode == 'full' || ($view_mode == 'teaser' && $settings->showteaser == 1)))) {
    // Never show edit link on view node.
    $settings->showeditlink = FALSE;
    $year = date('Y');
    $month = date('m');
    $node->content['availability_calendars'] = array(
      '#theme' => 'availability_calendars_node',
      '#node' => $calendar_node,
    	'#year' => $year,
      '#month' => $month,
      '#settings' => $settings,
      '#weight' => 10,
    );
  }
}

/**
 * Themes the given number of months of the calendar for the given node.
 *
 * @param array $variables
 * @return string
 */
function theme_availability_calendars_node($variables) {
  $node = $variables['node'];
  $year = &$variables['year'];
  $month = &$variables['month'];
  $settings = $variables['settings'];

  $output = '<div id="availability-calendar-' . $node->nid . '" class="availability-calendar clearfix">';

  // Create our key for the availability calendar if the node has it set to do so.
  if ($settings->showkey === 1) {
    $output .= theme('availability_calendars_key');
  }

  $monthsremaining = $settings->monthcount;
  while ($monthsremaining > 0) {
    $output .= theme('availability_calendars_month', $variables);
    $monthsremaining--;
    $month++;
    if ($month > 12) {
      $month = 1;
      $year++;
    }
  }

  $output .= '</div>';
  return $output;
}

/**
 * Themes the calendar for a given month.
 *
 * @param object $node
 * @param int $year
 * @param int $month
 * @param object $settings
 * @return string
 */
function theme_availability_calendars_month($variables) {
  // Calendar code based on example at http://evolt.org/node/60673 :
  $node = $variables['node'];
  $year = $variables['year'];
  $month = sprintf('%02d', $variables['month']);
  $settings = $variables['settings'];

  $month_meta = availability_calendars_month_meta($year, $month, $settings);
  /**
   * Here we list all the days of the week, an array of 14 (two full weeks) so
   * that if users select monday as the first day we still get a full week in
   * our following loop.
   */
  $days = array(
    13 => t('Mon'),
    12 => t('Tue'),
    11 => t('Wed'),
    10 => t('Thu'),
    9 => t('Fri'),
    8 => t('Sat'),
    7 => t('Sun'),
    6 => t('Mon'),
    5 => t('Tue'),
    4 => t('Wed'),
    3 => t('Thu'),
    2 => t('Fri'),
    1 => t('Sat'),
    0 => t('Sun'),
  );
  $counter = -$month_meta['firstday'];
  for ($j = 0; $j < $month_meta['weeksinmonth']; $j++) {
    for ($i = 0; $i < 7; $i++) {
      $counter++;
      $week[$j][$i] = $counter;
      // Offset the days.
      if (($week[$j][$i] < 1) || ($week[$j][$i] > $month_meta['daysinmonth'])) {
        $week[$j][$i] = "";
      }
    }
  }
  $month_title = format_date(mktime(12, 0, 0, $month, 1, $year), 'custom', 'F Y');
  if ($settings->showeditlink && availability_calendars_can_edit($node)) {
    // Add edit link to month_title.
    $month_title .=  ' ' . l(t('edit'), 'availability-calendars/' . $node->nid . '/' . date('Y/m', mktime(0, 0, 0, $month, 1, $year)) . '/edit', array('query' => array('destination' => 'node/' . $node->nid), 'attributes' => array('title' => t('Click to edit week notes and/or statuses per day'))));
  }
  $headers = array(); // Container for header row.
  if ($settings->showweeknotes) {
    array_push($headers, array('data' => '&nbsp;', 'class' => array('calempty'))); // Add one empty cell for the notes column.
  }
  // Add a header row showing the day of the week, we do some odd backwards looping through this...
  // Because the options in the node's form are set in a way that requires it.
  for ($i = $settings->startofweek + 7; $i > $settings->startofweek && $i <= (7 + $settings->startofweek); $i--) {
    $day = $settings->firstletter == 0 ? $days[$i] : drupal_substr($days[$i], 0, 1);
    array_push($headers, $day);
  }

  // Find all entries in database for this month ($availability, $notes) and pre-populate.
  $notes = availability_calendars_get_node_notes($node->nid, $year, $month);
  $states = availability_calendars_get_node_states($node->nid, $year, $month, $settings);

  $today = date(AC_ISODATE);
  $rows = array(); // Our container for rows.
  $cells = array(); // Our container for cells.
  foreach ($week as $key => $val) {
    $weeknumber = $key + 1;
    // Add the week note cell to the cells array
    if ($settings->showweeknotes) {
      array_push($cells, array('data' => filter_xss($notes[$weeknumber]), 'header' => TRUE));
    }
    for ($i = 0; $i < 7; $i++) {
      $day = $week[$key][$i];
      // If there's a date, it's part of this month.
      if ($day) {
        $daystamp = date(AC_ISODATE, mktime(0, 0, 0, $month, $day, $year));
        $status = $states[$daystamp];
        $classes = array();
        if ($daystamp < $today) {
          $classes[] = 'calpastdate';
          // Show or hide old states?
          if ($settings->hideold != 1) {
            $classes[] = $status;
          }
        }
        else {
          if ($today == $daystamp) {
            $classes[] = 'caltoday'; // Today
          }
          $classes[] = $status;
        }
        $day_cell = availability_calenders_day($day, $settings);
        array_push($cells, array('data' => $day_cell, 'class' => $classes));
      }
      else { // Empty day, beginning of row 1 or end of row 5/6 in a month.
        array_push($cells, array('data' => '<div></div>', 'class' => array('calother')));
      }
    }
    array_push($rows, array('data' => $cells, 'class' => array('calweek')));
    $cells = array(); // Clear out our $cells array before running the next week.
  }
  if ($weeknumber == 5) {
    if ($settings->showweeknotes) {
      array_push($cells, array('data' => '&nbsp;', 'header' => TRUE)); // Add the week note cell to the cells array.
    }
    for ($i = 0; $i < 7; $i++) {
      array_push($cells, array('data' => '<div></div>', 'class' => array('calother')));
    }
    array_push($rows, array('data' => $cells));
  }
  $output = theme('table', array('caption' => $month_title, 'header' => $headers, 'rows' => $rows, 'attributes' => array('id' => "cal-$year-$month-{$node->nid}", 'class' => array('cal', 'calmonth')), 'sticky' => FALSE)); // Our final table
  // Wrap the table to allow for better styling.
  return '<div class="calmonth-wrapper">' . $output . '</div>';
}

/**
 * Helper function that returns the html for 1 day.
 *
 * @param int $day The day number.
 * @param object $settings settings (for the current node).
 */
function availability_calenders_day($day, $settings) {
  // Represent whole days with 1 div, split ays woth 2 span's.
  // This allows to generate all css at once and then just switch settings
  if (!$settings->splitday) {
    // Whole day, the background and border color will take care of the coloring.
    // Border color may change on hover for "selectable" cells.
    $result = "<div>$day</div>";
  }
  else {
    // Split day:
    // - The border-color of the outer span will take care of the coloring.
    //    the outer span will have zero sized content and full sized borders.
    // - The inner span will contain the day number and the selectable border
    //   It will be positioned absolute, half way up and left.
    // However, to be able to position it absolute, it needs to be within another
    // positioned element, and td's cannot be positioned.
    $result = "<span><span>$day</span></span>";
  }
  return $result;
}

/**
 * Themes the key for our calendars.
 *
 * @return string
 */
function theme_availability_calendars_key($variables) {
  // Use all the same classes for cells and table, so it styles the same as the calendars.
  $keytitle = t('Key');
  $headers = array(
    t('Availability')
  );
  $rows = array();
  $states = availability_calendars_get_states();
  foreach ($states as $class => $state) {
    $rows[] = array(
      array(
        'data' => '<span>' . check_plain(t($state['label'])) . '</span>',
        'class' => "$class keystatus", // Keystatus class used to allow better styling of the status column.
      ),
    );
  }
  $rows[] = array(
    array(
      'data' => '<span>' . t('Current Day') . '</span>',
      'class' => 'caltoday keystatus',
    ),
  );
  $key = theme('table', array('caption' => $keytitle, 'header' => $headers, 'rows' => $rows, 'attributes' => array('class' => array('cal', 'calkey')), 'sticky' => FALSE));
  return '<div class="calmonth-wrapper key-wrapper">' . $key . '</div>';
}
