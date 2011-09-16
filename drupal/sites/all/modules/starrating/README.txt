/* README.txt */

Star Rating is a simple module that provides star rating field and display formatter using Drupal 7's Field Type API.

Unlike Fivestar module and other voting/rating module, it supports neither end user voting nor nice AJAX interface. It is inteded for the rating only by the author of the article (node). Of course, you may be able to do this with Fivestar module and other voting/rating modules, but unlike most of them, it does not require other modules such as voting API module. It's born to be simple but good enough for those who want star rating only by authors.

For example, if you are creating a restaurant review web site and you want to add three 5 star ratings (food, price and service) with different icon type for each rating within a node, then this module would be the best match.


Features
========
* Very simple and no other modules required.
* Utilizes Drupal 7's Field API.
* Fully supports Views module.
* Different icon type and color can be assigned for each field and each view mode.
  (Total 16 icon type x 8 colors = 128 different icons)


Installation
============
Just like any other modules, extract the archive at
(Drupal-root)/sites/all/modules directory. Then enable the module from Modules admin page atadmin/modules.


How to use
==========
To use Star Rating module, you need to create a field for the star rating. To do this, go to Content types admin page at admin/structure/types and edit/create a content type to which you want to add star rating field.

Go to 'Manage Field' tab and add a new filed with 'Star rating' field type selected. Then, the field settings tab is opened and you need to choose the maximum rating value for the field there. Once you press "Save field settings" button, 'Review settings' page is displayed and you can select the default rating value if you wish. Once the series of adding field steps is completed, go to the 'Manage Display' tab.
At 'Manage Display' tab, you can select some options including icon type and color to be used for the field. You can even assign different icon type and color for each display mode (default, full, teaser) if you wish.
That's all. When you create a new article and enter the rating value, you will see the rating icons of your choice instead of integer values.

Althogh the integer value of each field is replaced with icons and not displayed, the value is included and hidden in the HTML code so that the integer value is used when you copy and paste the web site contents to text editor, or when you use voice reading feature.


Known issue
===========
When you change the icon type and color from the manage display form, the sample with new icon type and color may not be displayed until you click on the 'Save Setting' button due to the issue of dynamic loading of a new CSS file.  I have no plan to fix this issue. Please let me know if you know a good way to deal with this issue.


Adding/changing icons
=====================
Currently, there's no UI for adding/changing icons. You need to manually replace icon image file (.png) and CSS file (.css) in the icons directory. For this, icon type 'Custom' is available. Currently, there are custom.css and custom.png file but they are place holders and identical with star.css and star.png. All you need to do is create your own custom.css and custom.png and replace the place holder files. Please take a moment and read the MEMO.txt file in the icons directory for additional information.


Author
======
Hideki Ito
PIXTURE STUDIO <http://www.pixture.com/>

