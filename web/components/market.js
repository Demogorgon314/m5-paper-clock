export const marketComponent = Object.freeze({
  type: "market",
  labelKey: "layout.component.summary",
  defaultItem: Object.freeze({
    id: "market-1",
    type: "market",
    labelKey: "layout.component.summary",
    x: 72,
    y: 392,
    w: 332,
    h: 86,
    props: Object.freeze({ variant: "summary-card", symbol: "sh000001" }),
  }),
  propsSchema: Object.freeze([
    Object.freeze({
      key: "symbol",
      type: "text",
      labelKey: "layout.prop.symbol",
      placeholderKey: "layout.prop.symbolPlaceholder",
    }),
  ]),
});

