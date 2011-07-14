(function($) {
/**
 * Helper method that extends the String object with a padLeft method
 */
String.prototype.padLeft = function(value, size) {
  var x = this;
  while (x.length < size) {
    x = value + x;
  }
  return x;
};

/**
 * Helper method that extends the Array object with a top method
 */
Array.prototype.top = function() {
  return this[this.length - 1];
};

/**
 * Helper method that extends the Date object with a toFormattedString method to allow for
 * easier printing of dates. (Reduced version of one that was found on Stackoverflow.)
 */
Date.prototype.toFormattedString = function(format) {
  return format
    .replace(/yyyy/g, this.getFullYear())
    .replace(/mm/g, String(this.getMonth() + 1).padLeft("0", 2))
    .replace(/m/g, String(this.getMonth() + 1))
    .replace(/dd/g, String(this.getDate()).padLeft("0", 2))
    .replace(/d/g, this.getDate());
};

Drupal.availabilityCalendars = {};
/**
 * Javascript API for availability Calendars module.
 *
 * @class AvailabilityCalendars
 *   Represents the client-side interface to an availability calendar
 *   for a given node. In principal, it is possible to have multiple calendars for differents
 *   nodes on the same page. If this is the case, use the nid parameter in the constructor to
 *   specify which calendar.
 *
 *   This API object provides a way of client-side interacting with the calendar. Basically it
 *   provides:
 *   - Some methods to change the (visual) status of calendar days. Note however, that it does not
 *     update the server-side calendar.
 *   - It defines a 'calendarclick' event and triggers it when the visitor clicks on a day cell in
 *     the calendar. The 'calendarclick' event passes in a Date object, representing the day that
 *     was clicked on, and the nid, to isdentify which calendar was clicked on. However, as only
 *     DOM elements can trigger events, you must bind your custom event handler to the calendar
 *     element, retrieved via AvailabilityCalendar.getCalendar():
 *     @code
 *     myAvailabilityCalendar.getCalendar().bind('calendarclick', function(event, date, nid) {
 *       alert('You clicked on date ' + date + ' of node ' + nid);
 *     });
 *     @endcode
 *
 * @constructor
 *   Creates a new AvailabilityCalendars interaction object.
 * @param object
 *   Settings Required: object with the following properties:
 *   {
 *     splitDay, (boolean: indicating whether this calendar allows split days)
 *     selectable, (string: none|all|available|not-available indicating whether this calendar
 *                  allows interaction by selecting dates)
 *     states {
 *       class* { (1 object per availability state)
 *         class,
 *         label,
 *         weight,
 *         is_available
 *       }
 *     }
 *   }
 * @param int nid
 *   The node id for the Calendar we want to interact with.
 */
Drupal.availabilityCalendars.Constructor = function(settings, nid) {
  var AM = 1;
  var PM = 2;
  var WHOLE_DAY = 3;
  var _calendarId = "#availability-calendar-" + nid;
  var _calendarRange = null;
  var _calendarDays = {};
  _init();

  /**
   * Initializes the calendar.
   * - Gives selectable cells the class selectable
   * - Initializes the custom calendarclick event on these cells
   */
  function _init() {
    _initEmptyState();
    _initDaysAdministration();
    _initSelectable();
    _initCustomEvents();
  };

  function _initEmptyState() {
    settings.states[""] = {is_available: false};
  }

  /**
   * Creates an overview of all days, their DOM element and their states
   */
  function _initDaysAdministration() {
    // Get all calendar months
    $(".calmonth")
      .filter(function() {
        // Make sure it is a calendar month
        return $(this).attr("id").split("-", 4).length == 4;
      })
      .each(function() {
        // Get year and month of this calendar month
        var day = null;
        var idParts = $(this).attr("id").split("-", 4);
        var year = idParts[1];
        var month = idParts[2];
        // Get all day cells of this calendar month
        $("tr.calweek > td", $(this))
          .not(".calother, .calpastdate")
          .each(function() {
            var cell = $(this);
            // Performance of the expresssion Number(cell.text()) is not best,
            // so use the fact that the set is ordered:
            // http://docs.jquery.com/Release:jQuery_1.3.2#Elements_Returned_in_Document_Order
            day = day === null ? Number(cell.text()) : day + 1;
            var date = (new Date(year, month - 1, day)).toFormattedString("yyyy-mm-dd");
            var currentClass = this.className;
            var state = {am: "", pm: ""};
            // Loop through all availability states (not including inherited properties)
            // to extract the availability state from the class.
            // Remove found classes to arrive at the other 'extra' classes.
            for (var key in settings.states) {
              if (settings.states.hasOwnProperty(key)) {
                var cssClass = settings.states[key].class;
                if (cell.hasClass(cssClass)) {
                  // Distribute over am and pm but do not overwrite.
                  state.am = state.am || cssClass;
                  state.pm = state.pm || cssClass;
                  cell.removeClass(cssClass);
                }
                if (cell.hasClass(cssClass + "-am")) {
                  state.am = cssClass;
                  cell.removeClass(cssClass + "-am");
                }
                if (cell.hasClass(cssClass + "-pm")) {
                  state.pm = cssClass;
                  cell.removeClass(cssClass + "-pm");
                }
              }
            }
            state = state.am === state.pm ? state.am : state;
            _calendarDays[date] = {cell: this, states: [state], extraStates: this.className};
            // Restore original class
            this.className = currentClass;
          });
      });
  };

  /**
   * Makes certain cells selectable.
   */
  function _initSelectable() {
    if (settings.selectable !== "none") {
      for (var day in _calendarDays) {
        switch (settings.selectable) {
          case "all":
            addExtraState(day, "calselectable");
            break;
          case "available":
            if (isAvailable(day, AM) || isAvailable(day, PM)) {
              addExtraState(day, "calselectable");
            }
            break;
          case "not-available":
            if (!isAvailable(day, AM) || !isAvailable(day, PM)) {
              addExtraState(day, "calselectable");
            }
            break;
        }
      }
    }
  };

  /**
   * Initializes the custom events for this calendar.
   *
   * The events are triggered by the calendar element, the div surrounding this calendar.
   * Other javascript thus should bind to that element (retrieved via @see getCalendar()).
   *
   * Currently provided custom events:
   * - calendarclick: comes with a date object and the nid as parameters.
   */
  function _initCustomEvents() {
    getCalendar().click(function(event) {
      // Find out if event originated from a day cell, get the date,
      // and trigger the event on the calendar element.
      var day, month, year;
      var cell = $(event.target).closest("td.calselectable");
      if (cell.size() > 0) {
        cell
          .each(function() {
            day = Number($(this).text());
          })
          .closest(".calmonth")
          .filter(function() {
            return $(this).attr("id").split("-", 4).length == 4;
          })
          .each(function() {
            var idParts = $(this).attr("id").split("-", 4);
            year = idParts[1];
            month = idParts[2];
          })
          .closest(_calendarId)
          .triggerHandler("calendarclick", [new Date(year, month - 1, day), nid]);
      }
    });
  };

  /**
   * @returns jQuery
   *   A jQuery object containing the calendar DOM element, that is the div
   *   with id='availability-calendar-{nid}' and class='availability-calendar' surrounding
   *   this calendar. This element triggers the custom events, thus other javascript should
   *   bind its calendar event handling to the return value of this method.
   */
  function getCalendar() {
    return $(_calendarId);
  };

  /**
   * @returns object
   *   An object containing the availability calendar settings.
   */
  function getSettings() {
    return settings;
  };

  /**
   * @returns object
   *   An object containing settings for all the availability states (indexed by the class).
   */
  function getStateSettings() {
    return settings.states;
  };

  /**
   * Returns the date range of the calendar.
   *
   * @returns Object
   *   { from: Date, to: Date }
   */
  function getCalendarRange() {
    if (_calendarRange === null) {
      var from, to;
      // Get all tables representing calendar months within *this* calendar,
      // extract the month and update the from/to range
      $(_calendarId + " table.calmonth").each(function() {
        var idParts = $(this).attr("id").split("-", 4);
        if (idParts.length == 4) {
          var year = idParts[1];
          var month = idParts[2];
          var calFrom = new Date(year, month - 1, 1);
          if (from === undefined || from > calFrom) {
            from = calFrom;
          }
          var calTo = new Date(year, month, 0); // 'They' say this works on all browsers (and should work according to the spec).
          if (to === undefined || to < calTo) {
            to = calTo;
          }
        }
      });
      _calendarRange = {from: from, to: to};
    }
    return _calendarRange;
  };

  /**
   * Returns whether the given date is within the calendar range.
   *
   * @param Date day
   * @returns Boolean
   */
  function isInCalendarRange(day) {
    var range = getCalendarRange();
    return range.from <= day && day <= range.to;
  };

  /**
   * Returns the number of months the calendar displays.
   *
   * @returns Integer
   */
  function getNumberOfMonths() {
    var range = getCalendarRange();
    return (range.to.getFullYear() - range.from.getFullYear()) * 12
      + range.to.getMonth() - range.from.getMonth() + 1;
  };

  /**
   * Returns the state for a given date.
   *
   * @param Date day
   * @returns null|String|Object
   *   A string for a whole day state or an object for a
   *   split day state: { state: String, am: String, pm: String } or
   *   null for a date not within the calendar range.
   */
  function getState(day) {
    var state = null;
    var date = typeof day === "string" ?  day : day.toFormattedString("yyyy-mm-dd");
    if (typeof _calendarDays[date] !== "undefined") {
      state = _calendarDays[date].states.top();
    }
    return state;
  };

  /**
   * Indicates whether a day state is to be considered available for the given dayPart
   *
   * @param String|Object state
   *   A single whole day state (String) or
   *   A composed am/pm split state (Object): { am: String, pm: String }
   * @param Integer dayPart optional (defaults to whole day)
   *   1 = am
   *   2 = pm
   *   3 = am + pm = whole day
   * @returns Boolean
   *   true if the state is available
   *   null when the state is not recognised.
   */
  function _isDayStateAvailable(state, dayPart) {
    var available;
    // Ignore dayPart if calendar does not support split days.
    if (dayPart === undefined || !settings.splitDay) {
      dayPart = WHOLE_DAY;
    }
    if (typeof state == "string") {
      available = getStateSettings()[state].is_available;
    }
    else {
      available = true;
      if ((dayPart & AM) !== 0) {
        // am state is to be taken into account.
        available = getStateSettings()[state.am].is_available;
      }
      if ((dayPart & PM) !== 0) {
        // pm state is to be taken into account.
        available = available && getStateSettings()[state.pm].is_available;
      }
    }
    return available;
  };

  /**
   * Returns whether the given day is available gfor the given dayPart
   *
   * @parma Date date
   * @param Integer dayPart optional (defaults to whole day)
   *   1 = am
   *   2 = pm
   *   3 = am + pm = whole day
   * @returns Boolean
   *   true if the (part of the) date is available.
   *   false if the (part of the) date is not available.
   *   null if the date is not within the calendar range.
   */
  function isAvailable(date, dayPart) {
    var available = null;
    var state = getState(date);
    if (state !== null) {
      available = _isDayStateAvailable(state, dayPart);
    }
    return available;
  };

  function _setCellClass(date) {
    var state = _calendarDays[date].states.top();
    var cssClass = typeof state === "string" ? state : state.am + "-am " + state.pm + "-pm";
    if (_calendarDays[date].extraStates !== '') {
      cssClass += " " + _calendarDays[date].extraStates;
    }
    _calendarDays[date].cell.className = cssClass;
  };

  /**
   * Changes the availability state of the given (part of the) day.
   *
   * @param Date day
   * @param String state
   * @param Integer dayPart optional (defaults to whole day)
   *   1 = am
   *   2 = pm
   *   3 = am + pm = whole day
   */
  function changeState(day, state, dayPart) {
    var date = typeof day === "string" ?  day : day.toFormattedString("yyyy-mm-dd");
    if (typeof _calendarDays[date] !== "undefined") {
      // Ignore dayPart if calendar does not support split days.
      if (dayPart === undefined || !settings.splitDay) {
        dayPart = WHOLE_DAY;
      }

      // Determine new composite state: get a clone of the current state ...
      var newState = _calendarDays[date].states.top();
      if (typeof newState === "string") {
        newState = {am: newState, pm: newState};
      }
      else {
        newState = {am: newState.am, pm: newState.pm};
      }
      // ... and overwrite the dayPart's to change
      if ((dayPart & AM) !== 0) {
        newState.am = state;
      }
      if ((dayPart & PM) !== 0) {
        newState.pm = state;
      }
      newState = newState.am === newState.pm ? newState.am : newState;
      _calendarDays[date].states.push(newState);
      // And set the class on the accompanying cell
      _setCellClass(date);
    }
  };

  /**
   * Restores the availability state of the given (part of the) day to its previous value.
   *
   * @param Date day
   */
  function restoreState(day) {
    var date = typeof day === "string" ?  day : day.toFormattedString("yyyy-mm-dd");
    if (typeof _calendarDays[date] !== "undefined") {
      // Remove current state (if not the original state)
      if (_calendarDays[date].states.length > 1) {
        _calendarDays[date].states.pop();
      }
      // And set the class on the accompanying cell
      _setCellClass(date);
    }
  };

  /**
   * Sets an extra state on the given (part of the) day.
   * Extra states do not mix with or replace the availability settings.
   *
   * @param Date|String day
   *   Date object or yyyy-mm-dd string
   * @param String extraState
   */
  function addExtraState(day, extraState) {
    var date = typeof day === "string" ?  day : day.toFormattedString("yyyy-mm-dd");
    if (typeof _calendarDays[date] !== "undefined") {
      // Add state
      if (_calendarDays[date].extraStates !== "") {
        _calendarDays[date].extraStates += " ";
      }
      _calendarDays[date].extraStates += extraState;
      // And set the class on the accompanying cell
      _setCellClass(date);
    }
  };

  /**
   * Removes an extra state of the given (part of the) day.
   * Extra states do not mix with or replace the availability settings.
   *
   * @param Date day
   * @param String state
   */
  function removeExtraState(day, extraState) {
    var date = typeof day === "string" ?  day : day.toFormattedString("yyyy-mm-dd");
    if (typeof _calendarDays[date] !== "undefined") {
      // Is state set?
      var extraStates = " " + _calendarDays[date].extraStates + " ";
      extraStates = extraStates.replace(" " + extraState + " ", " ");
      _calendarDays[date].extraStates = jQuery.trim(extraStates);
      // And set the class on the accompanying cell
      _setCellClass(date);
    }
  };

  /**
   * Returns whether all dates in the given range are available.
   * In the split day situation we check from 'from pm' to 'to am'.
   * In the whole day situation we check from 'from' up to but not including 'to'.
   *
   * @param Date from
   * @param Date to
   * @returns Boolean|null
   *   true if the whole range is available
   *   false if not the whole range is available
   *   null if the given date range is not fully within the calendar range.
   */
  function isRangeAvailable(from, to) {
    var available = null;
    if (!settings.splitDay) {
      // We don't have to check for the last day itself.
      to = new Date(to.getFullYear(), to.getMonth(), to.getDate() - 1);
    }
    if (isInCalendarRange(from) && isInCalendarRange(to)) {
      available = true;
      var day = from;
      var dayPart = PM;
      while (available && day <= to) {
        available = isAvailable(day, dayPart);
        day = new Date(day.getFullYear(), day.getMonth(), day.getDate() + 1);
        dayPart = day >= to ? AM : WHOLE_DAY;
      }
    }
    return available;
  };

  /**
   * Sets all days in the from - to range to the given state.
   * In the split day situation we change from 'from pm' to 'to am'.
   * In the whole day situation we change from 'from' up to but not including 'to'.
   *
   * @param Date from
   * @param Date to
   * @param String state
   */
  function changeRangeState(from, to, state) {
    if (!settings.splitDay) {
      // We don't have to change the last day itself.
      to = new Date(to.getFullYear(), to.getMonth(), to.getDate() - 1);
    }
    var calendarRange = getCalendarRange();
    if (calendarRange.from > from) {
      from = calendarRange.from;
    }
    if (calendarRange.to < to) {
      to = calendarRange.to;
    }
    var day = from;
    var dayPart = PM;
    while (day <= to) {
      changeState(day, state, dayPart);
      day = new Date(day.getFullYear(), day.getMonth(), day.getDate() + 1);
      dayPart = day >= to ? AM : WHOLE_DAY;
    }
  };

  /**
   * Restores all states for the days in the from - to range to their previous state.
   * In the split day situation we change from 'from' up to and including 'to'.
   * In the whole day situation we change from 'from' up to but not including 'to'.
   *
   * @param Date from
   * @param Date to
   */
  function restoreRangeState(from, to) {
    if (!settings.splitDay) {
      // We don't have to restore the last day itself.
      to = new Date(to.getFullYear(), to.getMonth(), to.getDate() - 1);
    }
    var calendarRange = getCalendarRange();
    if (calendarRange.from > from) {
      from = calendarRange.from;
    }
    if (calendarRange.to < to) {
      to = calendarRange.to;
    }
    var day = from;
    while (day <= to) {
      restoreState(day);
      day = new Date(day.getFullYear(), day.getMonth(), day.getDate() + 1);
    }
  };

  /**
   * Adds the given extra state to all days in the from - to range.
   * In the split day situation we change from 'from pm' to 'to am'.
   * In the whole day situation we change from 'from' up to but not including 'to'.
   * Extra states do not mix with or replace the availability settings.
   *
   * @param Date from
   * @param Date to
   * @param String state
   */
  function addRangeExtraState(from, to, state) {
    if (!settings.splitDay) {
      // We don't have to set the last day itself.
      to = new Date(to.getFullYear(), to.getMonth(), to.getDate() - 1);
    }
    var calendarRange = getCalendarRange();
    if (calendarRange.from > from) {
      from = calendarRange.from;
    }
    if (calendarRange.to < to) {
      to = calendarRange.to;
    }
    var day = from;
    while (day <= to) {
      addExtraState(day, state);
      day = new Date(day.getFullYear(), day.getMonth(), day.getDate() + 1);
    }
  };

  /**
   * Removes the given state from all days in the from - to range.
   * In the split day situation we remove from 'from pm' to 'to am'.
   * In the whole day situation we remove from 'from' up to but not including 'to'.
   * Extra states do not mix with or replace the availability settings.
   *
   * @param Date from
   * @param Date to
   * @param String state
   */
  function removeRangeExtraState(from, to, state) {
    if (!settings.splitDay) {
      // We don't have to set the last day itself.
      to = new Date(to.getFullYear(), to.getMonth(), to.getDate() - 1);
    }
    var calendarRange = getCalendarRange();
    if (calendarRange.from > from) {
      from = calendarRange.from;
    }
    if (calendarRange.to < to) {
      to = calendarRange.to;
    }
    var day = from;
    while (day <= to) {
      removeExtraState(day, state);
      day = new Date(day.getFullYear(), day.getMonth(), day.getDate() + 1);
    }
  };

  return {
    // Publicly exposed methods:
    getCalendar: getCalendar,
    getSettings: getSettings,
    getStateSettings: getStateSettings,
    getCalendarRange: getCalendarRange,
    isInCalendarRange: isInCalendarRange,
    getNumberOfMonths: getNumberOfMonths,
    getState: getState,
    isAvailable: isAvailable,
    changeState: changeState,
    restoreState: restoreState,
    addExtraState: addExtraState,
    removeExtraState: removeExtraState,
    isRangeAvailable: isRangeAvailable,
    changeRangeState: changeRangeState,
    restoreRangeState: restoreRangeState,
    addRangeExtraState: addRangeExtraState,
    removeRangeExtraState: removeRangeExtraState
  };
};

/**
 * @property object _calendars Collection of instances
 */
Drupal.availabilityCalendars._calendars = {};

/**
 * Multiton implementation for accessing calendars on the page
 */
Drupal.availabilityCalendars.get = function(nid, settings) {
  if (typeof Drupal.availabilityCalendars._calendars[nid] === 'undefined') {
    Drupal.availabilityCalendars._calendars[nid] = new Drupal.availabilityCalendars.Constructor(settings, nid);
  }
  return Drupal.availabilityCalendars._calendars[nid];
};

})(jQuery);
