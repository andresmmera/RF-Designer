# S-Parameter Viewer & RF Synthesis tools

## Introduction

This is a comprehensive toolkit for RF and microwave circuit design. It combines S-parameter visualization capabilities with synthesis tools for filters, matching networks, attenuators, and power combiners.

## Features

* Display features
    - Rectangular plot, Smith Chart and polar plot display
    - Markers display
    - Limit lines (only for the magnitude and phase plot)
* Reads Touchstone files and Qucs-S datasets
* Trace management
* Session management (save, load, and recent files listing features)
* RF circuit synthesis tools

## Interface overview

The tool interface is divided into two panels: 
* [Data display panel](/Data-display-panel/index)
    - Contains the display widgets for visualizing the data
* [Data management panel](/Data-management-panel/index)
    - Contains the widgets needed to managing the data
    - Marker display
    - Limit management
    - Synthesis tools
    - Note-taking widget

```{figure} /images/SPViewer-UI-layout-overview.png
---
class: with-border
---

Overview of the user interface.
```


### See more

```{toctree}
:maxdepth: 2

Data-display-panel/index
Data-management-panel/index