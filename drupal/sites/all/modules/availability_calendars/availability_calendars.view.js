/*
 * The code below serves as an example of how to enable and configure the viewport feature:
 * - This viewport shows 6 months (out of 12 or any other multiple of 3) in 2 rows of 3 months.
 * - It will slide down (or up) 1 row, being 3 months, per click.
 * - The month width and height properties indicate the width and height (in pixels) of a complete
 *   calendar month wrapper including its margins. If not available, style the width and height of
 *   the full outer viewport: .availability-calendars-viewport.
 * - Other styling applied to the viewport may typically include border, margin, and background.
 * - The buttons, with id's ac_up and ac_down, need to be created and styled by the custom code.
 * - However, the availability calendar viewport code will enable/disable these buttons, if passed
 *   in, using the disabled attribute. If your buttons are in fact anchors, the disabled attribute
 *   is strictly seen not allowed, but will still work fine, especially in combination with the
 *   CSS2 property cursor and CSS3 property pointer-events. But you can also "gray out" or even
 *   hide the buttons. Examples:
 *
#ac-up, #ac-down {
  cursor: pointer;
  background-color: #...;
  border: 1px solid #...;
}

#ac-up:hover, #ac-down:hover {
  background-color: #...;
}

#ac-up[disabled], #ac-down[disabled] {
  background-color: #...; // gray-out
  text-decoration: none;
  cursor: auto;
  pointer-events: none;
}
 *
 * - Copy, paste and adapt the following code to your custom PHP code (make sure you have the nid
 *   of the node available)
  // Add js script and settings for the viewport and the buttons
  $viewport_settings = array(
    'settings' => array(
      'month' => array('width' => 231, 'height' => 247),
      'viewport' => array('cols' => 3, 'rows' => 2),
      'step' => array('cols' => 0, 'rows' => 1)
    ),
    'backwardSelector' => '#ac-up',
    'forwardSelector' => '#ac-down',
  );
  availability_calendars_add_js('viewport', $viewport_settings, $node->nid);
 */

(function($) {
/**
 * @class Drupal.availabilityCalendars.Viewport
 *
 * @constructor Creates a new Drupal.availabilityCalendars.Viewport object.
 * @param object calendar
 *   Required: the calendar on which to operate.
 * @param object settings
 *   Optional: object with the following properties
 *   (name type (default) description):
 *   {
 *     month
 *       width     int (0) the width of 1 calendar month, used to calculate the width of the
 *                         viewport. If 0, use CSS to define the width of the viewport.
 *       height    int (0) the height of 1 calendar month, used to calculate the height of the
 *                         viewport. If 0, use CSS to define the height of the viewport.
 *     viewport
 *       cols      int (3) The number of months that is shown horizontally in the viewport.
 *       rows      int (2) The number of months that is shown vertically in the viewport.
 *     step
 *       cols      int (0) indicating how many columns of months to move horizontally on stepping.
 *       rows      int (1) indicating how many rows of months to move vertically on stepping.
 *     animate     Animation parameters, @see http://api.jquery.com/animate/
 *       backward  Advanced usage, do not pass in.
 *       forward   Advanced usage, do not pass in.
 *       speed     int|string ('slow') The animation speed, see jquery documentation of animate().
 *     totalMonths int (calculated) Advanced usage, do not pass in.
 *     firstMonth  int (calculated) Advanced usage, do not pass in.
 *   }
 * @param mixed backwardSelector
 *   Optional: any string or object that can be fed to the jquery()
 *   constructor function to return a jquery object and that defines the elements the user can
 *   click on to scroll the calendar backward. Typically this will be a selector string.
 * @param mixed forwardSelector
 *   Optional: like backwardSelector but this parameter defines the
 *   elements to scroll the calendar forward.
 */
Drupal.availabilityCalendars.Viewport = function(calendar, settings, backwardSelector, forwardSelector) {
   // Initialize settings.
  settings = $.extend(true, {
    month: {width: 0, height: 0},
    viewport: {cols: 3, rows: 2},
    step: {cols: 0, rows: 1},
    animate: {speed: 'slow'},
    totalMonths: calendar.getNumberOfMonths(),
    firstMonth: 1
  }, settings);
  if (!settings.animate.backward) {
    settings.animate.backward = {
        top: '+=' + (settings.step.rows * settings.month.height),
        left: '+=' + (settings.step.cols * settings.month.width)
    };
  };
  if (!settings.animate.forward) {
    settings.animate.forward = {
        top: '-=' + (settings.step.rows * settings.month.height),
        left: '-=' + (settings.step.cols * settings.month.width)
      };
  };

  // Wrap the viepwort around the calendar months (not including the key).
  // - An outer div with overflow: hidden to obscure the non visible months.
  // - An inner div that is positioned relative to move the visible part forward and backward
  //   within the viewport.
  // - If scrolling horizontally, we want the inner viewport to be infinitely wide.
  $('.calmonth-wrapper', calendar.getCalendar())
    .wrapAll('<div class="availability-calendars-viewport"><div class="availability-calendars-viewport-inner"></div></div>');
  // Initialize viewport dimensions (Only if passed in, if not passed in set this via CSS).
  var viewport = $('.availability-calendars-viewport', calendar.getCalendar());
  if (settings.month.width != 0) {
    viewport.width(settings.viewport.cols * settings.month.width);
  }
  if (settings.month.height != 0) {
    viewport.height(settings.viewport.rows * settings.month.height);
  }
  // Move to the inner viewport as that is the element to be animated.
  viewport = viewport.children();
  if (settings.step.cols != 0) {
    viewport.width(10000);
  }

  // Initialize the event handlers.
  if (backwardSelector) {
    $(backwardSelector).click(function() {
      scrollBackward();
    });
  }
  if (forwardSelector) {
    $(forwardSelector).click(function() {
      scrollForward();
    });
  }
  setEnabledState();

  /**
   * Set the enabled/disabled state of the clickable elements.
   * This function uses the disabled attribute. Though officially meant for form elements
   * (probably buttons in our cases), it can be used on a tags as well. In combination with
   * the CSS properties cursor and pointer-events (CSS3) you can visibly/effectively disable
   * an a tag:
   * a[disabled] { cursor: auto; pointer-events: none }
   */
  function setEnabledState() {
    if (settings.firstMonth <= 1) {
      $(backwardSelector).attr('disabled', 'disabled');
    }
    else {
      $(backwardSelector).removeAttr('disabled');
    }
    if (settings.firstMonth + settings.viewport.rows * settings.viewport.cols > settings.totalMonths) {
      $(forwardSelector).attr('disabled', 'disabled');
    }
    else {
      $(forwardSelector).removeAttr('disabled');
    }
  }

  /**
   * Scroll the viewport backward (if possible).
   */
  function scrollBackward() {
    if (settings.firstMonth > 1) {
      viewport.animate(settings.animate.backward, settings.animate.speed);
      settings.firstMonth -= settings.step.rows * settings.viewport.cols + settings.step.cols * settings.viewport.rows;
      setEnabledState();
    }
  };

  /**
   * Scroll the viewport forward (if possible).
   */
  function scrollForward() {
    if (settings.firstMonth + settings.viewport.rows * settings.viewport.cols <= settings.totalMonths) {
      viewport.animate(settings.animate.forward, settings.animate.speed);
      settings.firstMonth += settings.step.rows * settings.viewport.cols + settings.step.cols * settings.viewport.rows;
      setEnabledState();
    }
  };

  return {
    // Publicly exposed methods:
    scrollBackward: scrollBackward,
    scrollForward: scrollForward
  };
};
})(jQuery);
