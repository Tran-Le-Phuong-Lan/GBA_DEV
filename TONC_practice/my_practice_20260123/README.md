# ATTENTION

- **when export tile design with indexed colored from GIMP to `.png`**, in order to have the correct indexed colored converted by `grit`, the following options must be set 

    - The first index color (i.e index 0) must be black = the default transparent background color in GBA. Otherwise, any other color (not black) at index 0, when being loaded into GBA, it will appear to be black on the screen.

    - Export (as default) :Pixel format = Automatic, No selection is checked. Then the data converted by `grit` will follow exact color index as shown in GIMP.