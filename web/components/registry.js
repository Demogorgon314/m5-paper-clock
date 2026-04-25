import { batteryComponent } from "./battery.js";
import { calendarComponent } from "./calendar.js";
import { classicInfoComponent } from "./classicInfo.js";
import { classicTimeComponent } from "./classicTime.js";
import { climateComponent } from "./climate.js";
import { dateComponent } from "./date.js";
import { marketComponent } from "./market.js";
import { timeComponent } from "./time.js";

export const COMPONENT_DEFINITIONS = Object.freeze([
  dateComponent,
  batteryComponent,
  calendarComponent,
  timeComponent,
  marketComponent,
  climateComponent,
  classicTimeComponent,
  classicInfoComponent,
]);

export const COMPONENT_REGISTRY = Object.freeze(
  Object.fromEntries(
    COMPONENT_DEFINITIONS.map((definition) => [definition.type, definition]),
  ),
);

export const DEFAULT_DASHBOARD_LAYOUT = Object.freeze(
  COMPONENT_DEFINITIONS.map((definition) => definition.defaultItem),
);

export const DEFAULT_CLASSIC_LAYOUT = Object.freeze([
  Object.freeze({
    ...dateComponent.defaultItem,
    id: "classic-date",
    props: Object.freeze({
      ...dateComponent.defaultItem.props,
      variant: "classic-header",
    }),
  }),
  Object.freeze({
    ...batteryComponent.defaultItem,
    id: "classic-battery",
    props: Object.freeze({ variant: "classic-status" }),
  }),
  Object.freeze({
    id: classicTimeComponent.defaultItem.id,
    type: classicTimeComponent.defaultItem.type,
    labelKey: "layout.component.time",
    x: classicTimeComponent.defaultItem.x,
    y: classicTimeComponent.defaultItem.y,
    w: classicTimeComponent.defaultItem.w,
    h: classicTimeComponent.defaultItem.h,
    props: classicTimeComponent.defaultItem.props,
  }),
  Object.freeze({
    id: classicInfoComponent.defaultItem.id,
    type: classicInfoComponent.defaultItem.type,
    labelKey: "layout.component.climate",
    x: classicInfoComponent.defaultItem.x,
    y: classicInfoComponent.defaultItem.y,
    w: classicInfoComponent.defaultItem.w,
    h: classicInfoComponent.defaultItem.h,
    props: classicInfoComponent.defaultItem.props,
  }),
]);
