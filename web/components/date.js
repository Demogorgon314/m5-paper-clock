export const dateComponent = Object.freeze({
  type: "date",
  labelKey: "layout.component.date",
  defaultItem: Object.freeze({
    id: "date-main",
    type: "date",
    labelKey: "layout.component.date",
    x: 24,
    y: 20,
    w: 744,
    h: 44,
    props: Object.freeze({
      variant: "header",
      dateFormat: "yyyy-mm-dd",
      weekdayFormat: "short",
      showHoliday: true,
      layoutStyle: "inline",
      textSize: "7",
    }),
  }),
  propsSchema: Object.freeze([
    Object.freeze({
      key: "dateFormat",
      type: "select",
      labelKey: "layout.prop.dateFormat",
      options: Object.freeze([
        Object.freeze({ value: "yyyy-mm-dd", labelKey: "layout.dateFormat.yyyyMmDd" }),
        Object.freeze({ value: "yyyy/mm/dd", labelKey: "layout.dateFormat.yyyySlashMmDd" }),
        Object.freeze({ value: "mm-dd", labelKey: "layout.dateFormat.mmDd" }),
        Object.freeze({ value: "yyyy年m月d日", labelKey: "layout.dateFormat.zhLong" }),
      ]),
    }),
    Object.freeze({
      key: "weekdayFormat",
      type: "select",
      labelKey: "layout.prop.weekdayFormat",
      options: Object.freeze([
        Object.freeze({ value: "short", labelKey: "layout.weekdayFormat.short" }),
        Object.freeze({ value: "long", labelKey: "layout.weekdayFormat.long" }),
        Object.freeze({ value: "narrow", labelKey: "layout.weekdayFormat.narrow" }),
        Object.freeze({ value: "english-short", labelKey: "layout.weekdayFormat.englishShort" }),
      ]),
    }),
    Object.freeze({
      key: "layoutStyle",
      type: "select",
      labelKey: "layout.prop.dateLayout",
      options: Object.freeze([
        Object.freeze({ value: "inline", labelKey: "layout.dateLayout.inline" }),
        Object.freeze({ value: "two-line", labelKey: "layout.dateLayout.twoLine" }),
        Object.freeze({ value: "date-only", labelKey: "layout.dateLayout.dateOnly" }),
        Object.freeze({ value: "holiday-only", labelKey: "layout.dateLayout.holidayOnly" }),
      ]),
    }),
    Object.freeze({
      key: "showHoliday",
      type: "checkbox",
      labelKey: "layout.prop.showHoliday",
    }),
    Object.freeze({
      key: "textSize",
      type: "select",
      labelKey: "layout.prop.dateTextSize",
      options: Object.freeze([
        Object.freeze({ value: "2", labelKey: "layout.dateTextSize.small" }),
        Object.freeze({ value: "7", labelKey: "layout.dateTextSize.default" }),
        Object.freeze({ value: "3", labelKey: "layout.dateTextSize.large" }),
      ]),
    }),
  ]),
});
