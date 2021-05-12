DEBUG = false;

board_size = [1.5, 50, 80];
esp_size = [5, 51.8, 28.5];
hole_dia = 2;
inner_box = [45, 55, 82];
fatness = 3.6;
cable_hole_radius = 3.5;
cover_overlay = 10;
bottom_size = inner_box.z / 2 ;
top_size = inner_box.z - bottom_size;
board_thickens = 1.5;
rail_size = board_thickens + 0.2;
rail_thicknes = 1;
round_prec = 40;
led_hole_radius = 5.2;
acc_hole = 2.8;
power_hole_radius = 6;

echo("Box side thicknes ", fatness / 2);

module Board(x, y, z) {
    translate([x, y, z]) union() {
        color("black") cube(board_size);
    }
}

module ESP(x, y, z) {
    translate([x, y, z]) union() {
        color("black") cube(esp_size);
    }
}

module SideRail(x, y, z, length) {
    color("blue") translate([x, y, z]) difference() {
        cube([(rail_size * 2), rail_thicknes, length]);
        translate([rail_size / 2, 0, 0]) {
            cube([rail_size + .01, rail_thicknes + 0.1, length + 0.1]);
        }
    }
}

module BottomRail(x, y, z, length) {
    color("red") translate([x, y, z]) difference() {
        cube([(rail_size * 2), 5, length]);
        translate([rail_size / 2, 0, 0]) {
            cube([rail_size + .01, 5 + 0.1, length + 0.1]);
        }
    }
}

module PowerHole(x, y, z) {
    color("blue") rotate([0, 0, 0]) translate([x, y, z]) {
        cylinder(h = fatness + .02, d = power_hole_radius, $fn = round_prec);
    }
}

module LedHole(x, y, z) {
    color("blue") rotate([90, 0, 90]) translate([y, z, x]) {
        cylinder(h = fatness + .02, d = led_hole_radius, $fn = round_prec);
    }
}


module CableHole(x, y, z) {
    color("blue") rotate([90, 0, 90]) translate([y - fatness, z, x]) {
        translate([- cable_hole_radius / 2, 0, 0]) cube([cable_hole_radius, 100, fatness + .02]);
        cylinder(h = fatness + .02, d = cable_hole_radius, $fn = round_prec);
    }
}

module Bottom(x, y, z) {
    translate([x, y, z]) {
        difference() {
            union() {
                size = [inner_box.x + fatness, inner_box.y + fatness, (bottom_size + (fatness / 2))];
                cube(size);
                echo("Bottom size ", size);
                echo("Bottom overlay thicknes", (fatness / 2) - (fatness / 4));
                color("purple") translate([fatness / 4, fatness / 4, bottom_size + (fatness / 2)]) {
                    size = [inner_box.x + 2 * (fatness / 4), inner_box.y + 2 * (fatness / 4), cover_overlay];
                    echo("Bottom overlay ", size);
                    echo("Top overlay thicknes ", (fatness / 2) - (fatness / 4));
                    cube(size);
                }
            }
            translate([fatness / 2, fatness / 2, fatness / 2]) {
                color("gray") cube([inner_box.x, inner_box.y, bottom_size + 100]);
            }
            CableHole(inner_box.x + fatness / 2 - .01, inner_box.y - 7, 40);
            LedHole(inner_box.x + fatness / 2 - .01, fatness / 2 + inner_box.y / 2, 40);
            PowerHole(fatness / 2 + inner_box.x / 2, fatness / 2 + inner_box.y / 2, 0 - .01);
        }

        color("green") union() {
            translate([((inner_box.x / 5) - rail_size / 2), fatness / 2, fatness]) cube([rail_size * 2, 2.01,
                bottom_size]);
            SideRail(((inner_box.x / 5) - rail_size / 2), ((fatness / 2) + 2) - .01, z + fatness, bottom_size +
                .01)
            ;
            translate([(inner_box.x / 5) - rail_size / 2, (fatness / 2), fatness / 2]) cube([rail_size * 2, 12, 2]);
            rotate([90, 0, 0]) BottomRail(x + (inner_box.x / 5) - rail_size / 2, fatness / 2 - .01 + 2, - (12 + fatness
                / 2)
            , 12);
        }

        color("green") union() {
            translate([((inner_box.x / 5) - rail_size / 2), ((fatness / 2) + inner_box.y) - 3, fatness / 2 + 2])
                cube([rail_size * 2, 3.01, 7]);
            SideRail(((inner_box.x / 5) - rail_size / 2), (fatness / 2 + inner_box.y) - rail_thicknes - 3, z +
                    fatness / 2 + 7 - .01, 2);
            translate([(inner_box.x / 5) - rail_size / 2, (inner_box.y + fatness / 2) - 12, fatness / 2]) cube([
                    rail_size * 2, 12, 2]);
            rotate([90, 0, 0]) BottomRail((inner_box.x / 5) - rail_size / 2, fatness / 2 - .01 + 2, - (inner_box.y +
                    fatness / 2), 12);
        }


        color("pink") translate([(inner_box.x / 10) + 32, .01, fatness / 2]) cube([1, inner_box.y + fatness - .02, 33]);
        translate([inner_box.x - 15, fatness / 2 + 3, fatness / 2 - .01]) {
            roof_len = 22;
            acc_space = (roof_len - 15) / 2;
            echo("Acc space ", acc_space)
            color("white") translate([0, 0, 0]) {
                union() {
                    cylinder(h = 3, d = acc_hole, $fn = round_prec);
                    translate([0, 15, 0]) cylinder(h = 3, d = acc_hole, $fn = round_prec);
                }
            };
        }
        if (DEBUG) Board((inner_box.x / 5), fatness / 2 + 2, fatness / 2);
    }
}

module Cover(x, y, z) {
    translate([x, y, z]) {
        union() {
            difference() {
                size = [inner_box.x + fatness, inner_box.y + fatness, (top_size + (fatness / 2))];
                echo("Top size ", size);
                cube(size);
                translate([fatness / 2, fatness / 2, fatness / 2]) {
                    color("gray") cube([inner_box.x, inner_box.y, top_size + 100]);
                }
                taper_start = top_size - cover_overlay + fatness / 2 + .01;
                echo("Lower edge of taper ", taper_start, " taper height ", top_size - taper_start);
                translate([fatness / 4 - .1, fatness / 4 - .1, taper_start]) {
                    size = [inner_box.x + 2 * (fatness / 4 + .1), inner_box.y + 2 * (fatness / 4 + .1), cover_overlay];
                    echo("Top overlay thicknes ", size);
                    echo("Top overlay thicknes ", (fatness / 2) - (fatness / 4 - .1));
                    color("purple") cube(size);
                }
            }
        }
    }
}


Bottom(0, 0, 0);
Cover(100, 0, 0);
if (DEBUG) ESP(fatness / 2 + 40, (inner_box.y - esp_size.y) / 2, fatness / 2);
