# Docking extension for wxWidgets<br>
## Written by Gerhard W. Gruber &lt;sparhawk@gmx.at&gt; in 2021<br>
<br>

## Design<br>

The wxDocking components are an extension for the wxWidgets library, to add support for designing a modern look and feel. The goal was, to have a full set of API functions which allows the user a maximum of freedom while at the same time minimizes the need for new components or requires the user to use replacements for already established components.<br>
wxDocking was implemented by making use of existing components, so that it should work on most platforms supported by wxWidgets without special, platform dependent code. 
<br>
<br>
<br>
### TODO/Enhancements<br>

* Fix Add tab in bottom splitter of demo tab screws up<br>
* Refactor code to make smaller functions<br>
* Move windows via API<br>
* Highlight docking target and show selector for border or notebook<br>
* Add support for toolbar<br>
* Movement of toolbar ribbons (maybe should be part of wxToolbar?)<br>
* Save layout, use a callback mechanism so that the user can add custom information<br>
* Restore layout, use a callback mechansim so that the user can create windows as required<br>
* Floating windows (multiple separate frames)<br>
* Tabcontrol movement (maybe should be part of wxNotebbok?)<br>
* Toolbars<br>
* Toolbar ribbon movement (maybe should be part of wxToolbar?)<br>
* Events for handling docking events<br>
* * Start docking(veto)<br>
* * End docking, can not be vetoed because when the user releases the mouse button, that's it<br>
* * Allow docking (veto)<br>
* * Drag docking (veto) If vetoed, has to prevent repeated start events<br>
* * Panel selected(Same as allow docking?)<br>
* * Show selectors (veto)<br>
* * Hide selectors<br>
* * Create panel<br>
* * Destroy panel<br>
* Apply wxWidgets coding guidelines https://wxwidgets.org/develop/coding-guidelines/ and specifically https://wxwidgets.org/develop/coding-guidelines/#use_cpp_ext<br>
* <s>Create layout via API</s><br>
* <s>Fix resize problem when adding splitters</s><br>
* <s>Fix Add tab in bottom splitter of demo tab screws up</s><br>
* <s>Move windows via API</s><br>
