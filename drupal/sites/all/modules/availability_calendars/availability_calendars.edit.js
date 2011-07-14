(function($) {
/**
 * @class Drupal.availabilityCalendars.Command represents a calendar state changing command
 * during the whole creation phase, i.e. from click on a state to the click on the end date.
 */
Drupal.availabilityCalendars.Command = function() {
  this.state = '';
  this.from = null;
  this.to = null;

  /**
   * Sets the state of the current command and, as this is supposed to be the first parameter
   * to be set, cleans the from and to dates.
   */
  this.setState = function(selectedState) {
    if (selectedState !== undefined) {
      this.state = selectedState;
    }
    this.from = null;
    this.to = null;
    this.show();
  };

  /**
   * Adds a date to the command. If it is the first date it will be the from date. If it
   * is the 2nd date it will be the to date, if necessary, swapping the from and to dates.
   */
  this.addDate = function(date) {
    if (this.from === null) {
      this.from = date;
    }
    else if (this.to === null) {
      this.to = date;
      if (this.to < this.from) {
        var _from = this.from;
        this.from = this.to;
        this.to = _from;
      }
    }
    this.show();
  };

  /**
   * @returns Boolean Whether the current command is complete.
   */
  this.isComplete = function() {
    return this.to !== null && this.from !== null & this.state !== '';
  };

  /**
   * Shows (and replaces) the current command in the accompanying textarea.
   */
  this.show = function(splitDay) {
    var val = $('#edit-availability-changes').val();
    var pos = val.lastIndexOf('\n') + 1;
    val = val.substr(0, pos) + this.toString(splitDay);
    $('#edit-availability-changes').val(val);
  };

  /**
   * Finishes the current command and starts a new one.
   */
  this.finish = function(splitDay) {
    this.show(splitDay);
    $('#edit-availability-changes').val($('#edit-availability-changes').val() + '\n');
    this.setState();
  };

  /**
   * Removes the last incomplete command (needed just before submitting to prevent validation
   * errors).
   */
  this.remove = function() {
    var val = $('#edit-availability-changes').val();
    var pos = val.lastIndexOf('\n') + 1;
    val = val.substr(0, pos);
    $('#edit-availability-changes').val(val);
  };

  /**
   * @returns String
   *   A string representation of the current command.
   */
  this.toString = function(splitDay) {
    result = '';
    result += 'state: ';
    result += this.state !== '' ? this.state : '-';
    result += ' from: ';
    result += this.from !== null ? this.from.toFormattedString('yyyy-mm-dd') : '-';
    result += this.from !== null && splitDay && (this.to === null || this.from.valueOf() !== this.to.valueOf()) ? '(pm)' : '';
    result += ' to: ';
    result += this.to !== null ? this.to.toFormattedString('yyyy-mm-dd') : '-';
    result += this.to !== null && splitDay && (this.from == null || this.from.valueOf() !== this.to.valueOf())? '(am)' : '';
    return result;
  };
};

/**
 * @class Drupal.availabilityCalendars.Edit provides the glueing code that connects
 * the form elements on the /node/{nid}/edit-availability-calendar pages with the
 * @see AvailabilityCalendars API object and the @see Drupal.availabilityCalendars.Command class.
 */
Drupal.availabilityCalendars.Edit = function(calendar) {
  var command = new Drupal.availabilityCalendars.Command();
  var changed = false;

  // Initialize command and state radio-buttons
  command.setState($('#edit-availability-states input:radio:checked').val());
  $('#edit-availability-states input:radio').parent().addClass(function() {
    return $(this).children('input:radio:').val();
  });

  // Attach to state radios events.
  $('#edit-availability-states input:radio').click(function() {
    // State clicked: remove calselected (if set) and restart current command
    if (command.from !== null) {
      calendar.removeExtraState(command.from, 'calselected');
    }
    command.setState($('#edit-availability-states input:radio:checked').val());
  });

  // Attach to the calendar events.
  calendar.getCalendar().bind('calendarclick', function(event, date, nid) {
    command.addDate(date);
    if (!command.isComplete()) {
      calendar.addExtraState(command.from, 'calselected');
    }
    else {
      calendar.removeExtraState(command.from, 'calselected');
      calendar.removeExtraState(command.to, 'calselected');
      calendar.changeRangeState(command.from, command.to, command.state);
      command.finish(calendar.getSettings().splitDay);

      // Show warning.
      if (changed === false) {
        var warning = '<div class="messages warning"> ' + Drupal.t('Changes made to this calendar will not be saved until the form is submitted.') + '</div>';
        $(warning).insertBefore('#availability-calendars-node-edit-calendar-form .form-actions').hide().fadeIn('slow');
        changed = true;
      }
    }
  });

  // Attach to the submit button event.
  $('#edit-submit').click(function() {
    command.remove();
  });
  return null;
};

/**
 * Instantiates a @see Drupal.availabilityCalendars.Edit object on load.
 * It is assumed that only one calendar will be edited on a page. So only
 * 1 edit object will be created, thus this can be done in this file self.
 */
Drupal.behaviors.availabilityCalendarsEdit = {
  attach: function(context, settings) {
    Drupal.availabilityCalendars.editor = new Drupal.availabilityCalendars.Edit(
      Drupal.availabilityCalendars.get(settings.availabilityCalendars.edit.nid,
        settings.availabilityCalendars.edit.settings)
    );
  }
};
})(jQuery);
