export const climateComponent = Object.freeze({
  type: "climate",
  labelKey: "layout.component.climate",
  defaultItem: Object.freeze({
    id: "climate-main",
    type: "climate",
    labelKey: "layout.component.climate",
    x: 428,
    y: 392,
    w: 456,
    h: 86,
    props: Object.freeze({
      variant: "compact-card",
      comfortTemperatureMin: 19,
      comfortTemperatureMax: 27,
      comfortHumidityMin: 20,
      comfortHumidityMax: 85,
    }),
  }),
  propsSchema: Object.freeze([]),
});
