import re
from collections import defaultdict

class CParser:
    """
    Parses GUI-Guider C code to extract UI element information.
    """
    def __init__(self, c_code):
        self.c_code = c_code
        # Use a dictionary to store elements by name for easy lookup
        self.ui_elements = {}

    def parse(self):
        """
        Main parsing function.
        Uses regular expressions to find LVGL function calls and extract data.
        """
        print("Parsing C code...")

        # Regex for creating widgets: e.g., ui->screen_cont_1 = lv_obj_create(ui->screen);
        create_pattern = re.compile(r"\s*ui->(\w+)\s*=\s*lv_(\w+)_create\((.*?)\);")

        # Regex for setting properties: e.g., lv_obj_set_pos(ui->screen_cont_1, 0, 0);
        # This is a general pattern for various set functions.
        set_pattern = re.compile(r"\s*lv_(\w+)_set_(\w+)\((.*?)\);")
        
        # Regex for setting styles: e.g., lv_obj_set_style_bg_color(ui->screen_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
        style_pattern = re.compile(r"\s*lv_obj_set_style_(\w+)\((.*?)\);")

        # First pass: find all widget creations
        for match in create_pattern.finditer(self.c_code):
            widget_name = match.group(1)
            widget_type = match.group(2)
            params = [p.strip() for p in match.group(3).split(',')]
            parent = params[0].replace('ui->', '') if 'ui->' in params[0] else params[0]
            
            self.ui_elements[widget_name] = {
                'name': widget_name,
                'type': widget_type,
                'parent': parent,
                'properties': [],
                'styles': defaultdict(list)
            }

        # Second pass: find all property settings
        for match in set_pattern.finditer(self.c_code):
            target_widget_full = match.group(3).split(',')[0].strip()
            if 'ui->' in target_widget_full:
                target_widget_name = target_widget_full.replace('ui->', '')
                if target_widget_name in self.ui_elements:
                    prop_name = match.group(2)
                    params = [p.strip() for p in match.group(3).split(',')[1:]]
                    self.ui_elements[target_widget_name]['properties'].append({
                        'property': prop_name,
                        'params': params
                    })

        # Third pass: find all style settings
        for match in style_pattern.finditer(self.c_code):
            style_prop = match.group(1)
            params_str = match.group(2)
            params = [p.strip() for p in params_str.split(',')]
            
            target_widget_full = params[0]
            if 'ui->' in target_widget_full:
                target_widget_name = target_widget_full.replace('ui->', '')
                if target_widget_name in self.ui_elements:
                    value = params[1]
                    # The selector is usually the last parameter
                    selector = params[-1] if len(params) > 2 else "LV_PART_MAIN|LV_STATE_DEFAULT"
                    
                    self.ui_elements[target_widget_name]['styles'][selector].append({
                        'property': style_prop,
                        'value': value
                    })

        # Return the list of dictionary values
        return list(self.ui_elements.values())
