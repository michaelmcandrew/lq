/**
 * README for Availability Calendars module
 * @author Dan Karran (geodaniel) <dan at karran dot net>
 * @author Nicholas Alipaz (nicholas.alipaz)
 * @author Erwin Derksen (http://drupal.org/user/750928)
 */

NOTE: THIS INFORMATION IS OUTDATED AND NEEDS TO BE UPDATED TO REFLECT THE
      CURRENT FEATURES.

The availability calendars module allows for a site administrator to display
availability information on specified content types. You can choose the number
of months to show on each node as well as which day of the week to start the
calendars on for that node. Each week can have a short note displayed about it
(e.g. a rental price for that week) and each day can have a colour-coded status.

Installation
============

To install the Availability Calendars module you should:

 1. Extract the availability_calendars module (zip/tarball) into your
    sites/[sitename]/modules directory if it is just for use on a single site,
    or to the sites/all/modules directory if there is more than one site running
    on your Drupal installation that will be using this module.

 2. Enable the module under Administer -> Site building -> Modules.
 
 3. Setup permissions for your user roles under
    Administer -> User management -> Permissions.
      * 'edit availability calendars' allows a user role to edit all
        availability calendars on a site.
      * 'edit own availability calendars' allows a user role to only edit the
        availability on calendars attached to nodes they are the owner of.
 
 4. Enable availability calendars for any node type you wish (you may want to
    create a custom one content type) under Administer -> Content management ->
    Content types. The setting can be found in the 'Availability calendar
    settings' part of the form.
 
 5. Choose the global settings for the availability calendars under
    Administer -> Site configuration -> Availability calendars. You can choose
    how many months will be shown for each node and whether or not to display
    the calendars on the node page itself. Note that users with  enough
    permissions will see three extra months on nodes with availability calendars
    to allow them to set up the availability before those months are shown to
    users of the site. You can disable the display of calendars on nodes if you
    are comfortable with using the theming function to display them in another
    place (for example if you want to put all the availability calendars for
    your site on a single page).  There are additional settings here that you
    may want to consider setting to your preferences.

Setting availability information
================================

For each content type that has availability calendar support enabled, you will
see an 'Availability calendar settings' section on the node edit page where you
can change the following general option for the availability calendar on that
node:

  * First day of week
    This will be the day that shows first in each week of the calendars for that
    node.
  * Show a key for the availability calendars
  * Use only the first letter from the day of the week
  * Set all calendars to have their past dates shown as fully booked
   
When looking at a node that has availability calendars enabled, you can click
edit on any month you have the permission to edit, where you'll be able to set:

  * Note
    A note to show beside each week, for example a price.
   
  * Availability
    The status for each day.