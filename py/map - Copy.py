from pathlib import Path

import branca.colormap as bcm
import folium
import geopandas as gpd
import numpy as np
import pandas as pd
import streamlit as st
from shapely.geometry import Point, box
from shapely.ops import unary_union
from streamlit_folium import st_folium

from byteprocessor import load_grid

st.set_page_config(layout="wide", page_title="Binghamton Housing & Grid Explorer")
st.title("🗺️ Binghamton Housing & Grid Explorer")
st.markdown("Explore simulation grids, census tracts, and actual multifamily developments across Broome County.")

LABELS = {
    "zone": "Zone Type",
    "utility": "Utility Access Factor",
    "pop_factor": "Population Factor",
    "employment": "Employment Factor",
    "jobs": "Job Quantity",
    "rent": "Rent Factor",
    "quality": "Quality",
}
NUMERIC = [k for k in LABELS if k != "zone"]

ZONE_NAMES = {}
ZONE_COLORS = ["#4e79a7", "#f28e2b", "#e15759", "#76b7b2", "#59a14f", "#edc948", "#b07aa1"]

DEFAULT_FILE = Path(__file__).with_name("mapOutput.txt")
DEV_FILE = Path(__file__).with_name("broome_multifamily_developments_expanded(1).csv")
UTM_18N = 32618  # Binghamton UTM zone 18N (metres)

PLACE_URL = "https://www2.census.gov/geo/tiger/TIGER2023/PLACE/tl_2023_36_place.zip"
FALLBACK_BBOX = (-75.9700, 42.0650, -75.8800, 42.1300)


def zone_name(z):
    return ZONE_NAMES.get(int(z), f"Zone {int(z)}")


@st.cache_data(show_spinner=False)
def parse_grid(raw: bytes):
    return load_grid(raw)


@st.cache_data(show_spinner=False)
def load_developments():
    if DEV_FILE.exists():
        df = pd.read_csv(DEV_FILE)
        return df
    return pd.DataFrame()


@st.cache_data(show_spinner=False)
def load_city_boundary(src=PLACE_URL):
    places = gpd.read_file(src)
    city = places[places["NAMELSAD"] == "Binghamton city"]
    if city.empty:
        city = places[places["NAME"] == "Binghamton"]
    if city.empty:
        raise ValueError("Binghamton was not found in the places file.")
    return city[["NAMELSAD", "geometry"]].to_crs(UTM_18N)


@st.cache_data(show_spinner=False)
def build_grid(df, nx, ny, size_m, header_size_m, cx, cy):
    scale = size_m / header_size_m
    x0 = cx - nx * size_m / 2
    y0 = cy - ny * size_m / 2
    polys = [
        box(x0 + x * scale, y0 + y * scale, x0 + x * scale + size_m, y0 + y * scale + size_m)
        for x, y in zip(df["x_m"], df["y_m"])
    ]
    g = gpd.GeoDataFrame(df.copy(), geometry=polys, crs=UTM_18N).to_crs(4326)
    g["cell_id"] = g["col"].astype(str) + "," + g["row"].astype(str)
    g["zone_name"] = g["zone"].map(zone_name)
    for k in NUMERIC:
        g[k] = g[k].round(3)
    return g


# ---------------------------------------------------------------------------
# Load data
# ---------------------------------------------------------------------------
st.sidebar.header("Data Layers")
upload = st.sidebar.file_uploader("mapOutput file (optional)", type=None)

if upload is not None:
    raw = upload.getvalue()
elif DEFAULT_FILE.exists():
    raw = DEFAULT_FILE.read_bytes()
elif Path("mapOutput.txt").exists():
    raw = Path("mapOutput.txt").read_bytes()
else:
    st.info("Put `mapOutput.txt` next to this script, or upload it in the sidebar.")
    st.stop()

try:
    nx, ny, header_size_m, df = parse_grid(raw)
except Exception as e:
    st.error(f"Could not read the grid file: {e}")
    st.stop()

dev_df = load_developments()

# ---------------------------------------------------------------------------
# Sidebar controls
# ---------------------------------------------------------------------------
st.sidebar.header("Map Configurations")
attr = st.sidebar.selectbox("Attribute to visualize:", list(LABELS), format_func=LABELS.get)
cmap_name = st.sidebar.selectbox(
    "Color theme:", ["viridis", "plasma", "YlOrRd_09", "Blues_09", "YlGnBu_09", "PuRd_09", "RdYlGn_11"]
)
clip = st.sidebar.checkbox("Clip outliers (2nd-98th percentile)", value=True)
basemap = st.sidebar.radio("Base map:", ["Street", "Satellite"], horizontal=True)

show_developments = st.sidebar.checkbox("Show Multifamily Developments", value=True)

st.sidebar.header("Grid placement")

city = None
city_error = None
try:
    with st.spinner("Loading Binghamton city limits..."):
        city = load_city_boundary()
except Exception as e:
    city_error = e

placement = st.sidebar.radio("Placement:", ["Cover Binghamton", "Manual"], horizontal=True)

if placement == "Cover Binghamton":
    if city is not None:
        minx, miny, maxx, maxy = city.total_bounds
    else:
        minx, miny, maxx, maxy = (
            gpd.GeoSeries([box(*FALLBACK_BBOX)], crs=4326).to_crs(UTM_18N).total_bounds
        )
    size_m = max((maxx - minx) / nx, (maxy - miny) / ny) * 1.02
    cx, cy = (minx + maxx) / 2, (miny + maxy) / 2
else:
    size_m = st.sidebar.number_input(
        "Square size (m)", min_value=0.1, value=float(header_size_m), step=1.0,
    )
    lat = st.sidebar.number_input("Grid center latitude", value=42.0987, format="%.5f", step=0.0005)
    lon = st.sidebar.number_input("Grid center longitude", value=-75.9180, format="%.5f", step=0.0005)
    center = gpd.GeoSeries([Point(lon, lat)], crs=4326).to_crs(UTM_18N).iloc[0]
    cx, cy = center.x, center.y

gdf = build_grid(df, nx, ny, size_m, header_size_m, cx, cy)

city_ll = city.to_crs(4326) if city is not None else None
if city_ll is not None:
    only_city = st.sidebar.checkbox("Only show squares that touch the city limit", value=False)
    if only_city:
        gdf = gdf[gdf.intersects(unary_union(list(city_ll.geometry)))]
        if gdf.empty:
            st.warning("No squares touch the city limit.")
            st.stop()

# ---------------------------------------------------------------------------
# Grid-wide & Development Totals
# ---------------------------------------------------------------------------
m1, m2, m3, m4, m5 = st.columns(5)
m1.metric("Squares", f"{len(gdf):,}")
m2.metric("Total jobs", f"{gdf['jobs'].sum():,.1f}")
m3.metric("Total population factor", f"{gdf['pop_factor'].sum():,.1f}")
m4.metric("Avg rent factor", f"{gdf['rent'].mean():.3f}")
m5.metric("Multifamily Units", f"{dev_df['units'].sum():,.0f}" if not dev_df.empty else "N/A")

# ---------------------------------------------------------------------------
# Colour scale
# ---------------------------------------------------------------------------
if attr == "zone":
    zones = sorted(gdf["zone"].unique())
    colors = [ZONE_COLORS[i % len(ZONE_COLORS)] for i in range(len(zones))]
    lo, hi = min(zones), max(zones) + 1
    cmap = bcm.StepColormap(colors, vmin=lo, vmax=hi, index=list(range(lo, hi + 1))[: len(zones) + 1])
    cmap.caption = "Zone Type"
else:
    vals = gdf[attr]
    lo, hi = (np.percentile(vals, [2, 98]) if clip else (vals.min(), vals.max()))
    if hi <= lo:
        hi = lo + 1
    cmap = getattr(bcm.linear, cmap_name).scale(lo, hi)
    cmap.caption = LABELS[attr]


def color_for(value):
    if attr == "zone":
        return ZONE_COLORS[int(value) % len(ZONE_COLORS)]
    return cmap(min(max(value, lo), hi))


# ---------------------------------------------------------------------------
# Map
# ---------------------------------------------------------------------------
gminx, gminy, gmaxx, gmaxy = gdf.total_bounds
m = folium.Map(location=[(gminy + gmaxy) / 2, (gminx + gmaxx) / 2], zoom_start=12, tiles=None, max_zoom=22)
if basemap == "Street":
    folium.TileLayer("openstreetmap", max_native_zoom=19, max_zoom=22, name="Street").add_to(m)
else:
    folium.TileLayer(
        tiles="https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}",
        attr="Esri, Maxar, Earthstar Geographics",
        max_native_zoom=19,
        max_zoom=22,
        name="Satellite",
    ).add_to(m)

fields = ["cell_id", "zone_name"] + NUMERIC
aliases = ["Square (x,y):", "Zone:"] + [LABELS[k] + ":" for k in NUMERIC]

folium.GeoJson(
    gdf[["geometry"] + fields + ["zone"]],
    name="Grid",
    style_function=lambda f: {
        "fillColor": color_for(f["properties"][attr]),
        "color": "#222222",
        "weight": 0.6,
        "fillOpacity": 0.75,
    },
    highlight_function=lambda f: {"color": "#000000", "weight": 3, "fillOpacity": 0.9},
    tooltip=folium.GeoJsonTooltip(
        fields=["cell_id", "zone_name"] + ([attr] if attr != "zone" else []),
        aliases=["Square (x,y):", "Zone:"] + ([LABELS[attr] + ":"] if attr != "zone" else []),
        style="background-color: white; color: #333; font-family: arial; font-size: 12px; padding: 8px;",
    ),
    popup=folium.GeoJsonPopup(fields=fields, aliases=aliases, max_width=320),
).add_to(m)

if city_ll is not None:
    folium.GeoJson(
        city_ll[["geometry"]],
        name="Binghamton city limit",
        style_function=lambda f: {"color": "#d62728", "weight": 3, "fill": False, "dashArray": "6 4"},
    ).add_to(m)

# Add Multifamily Developments Layer
if show_developments and not dev_df.empty:
    dev_group = folium.FeatureGroup(name="Multifamily Developments")
    for _, row in dev_df.iterrows():
        lat_val = row.get("latitude_y")
        lon_val = row.get("longitude_x")
        if pd.notna(lat_val) and pd.notna(lon_val):
            popup_html = f"""
            <b>{row.get('development_name')}</b><br>
            Municipality: {row.get('municipality')}<br>
            Units: {row.get('units')}<br>
            Status: {row.get('status')}<br>
            Type: {row.get('development_type')}<br>
            Address: {row.get('site_address')}
            """
            folium.CircleMarker(
                location=[lat_val, lon_val],
                radius=max(4, min(12, np.sqrt(row.get('units', 50)))),
                color="#1f77b4",
                fill=True,
                fill_color="#ff7f0e",
                fill_opacity=0.8,
                weight=2,
                popup=folium.Popup(popup_html, max_width=300),
                tooltip=f"{row.get('development_name')} ({row.get('units')} units)"
            ).add_to(dev_group)
    dev_group.add_to(m)

m.fit_bounds([[gminy, gminx], [gmaxy, gmaxx]], max_zoom=21)
cmap.add_to(m)
folium.LayerControl().add_to(m)

# ---------------------------------------------------------------------------
# Layout: map + details
# ---------------------------------------------------------------------------
col_map, col_info = st.columns([3, 2])

with col_map:
    map_state = st_folium(
        m,
        height=640,
        use_container_width=True,
        key="grid_map",
        returned_objects=["last_active_drawing"],
    )

with col_info:
    clicked = (map_state or {}).get("last_active_drawing")
    cell = (clicked.get("properties") or {}).get("cell_id") if clicked else None

    if cell and (gdf["cell_id"] == cell).any():
        row = gdf[gdf["cell_id"] == cell].iloc[0]
        st.subheader(f"Square ({cell})")
        st.caption(f"{row['zone_name']}")

        a, b = st.columns(2)
        a.metric("Job quantity", f"{row['jobs']:.3f}", f"{row['jobs'] - gdf['jobs'].mean():+.3f} vs avg")
        b.metric("Population factor", f"{row['pop_factor']:.3f}", f"{row['pop_factor'] - gdf['pop_factor'].mean():+.3f} vs avg")
        a.metric("Rent factor", f"{row['rent']:.3f}", f"{row['rent'] - gdf['rent'].mean():+.3f} vs avg")
        b.metric("Utility access", f"{row['utility']:.3f}", f"{row['utility'] - gdf['utility'].mean():+.3f} vs avg")
        a.metric("Employment factor", f"{row['employment']:.3f}")
        b.metric("Quality", f"{row['quality']:.3f}")

        if attr != "zone":
            rank = int(gdf[attr].rank(ascending=False, method="min")[row.name])
            st.write(f"Ranks **#{rank} of {len(gdf)}** squares by {LABELS[attr].lower()}.")
        st.divider()
    else:
        st.info("Click a grid square on the map to inspect its metrics.")

    st.subheader("Broome County Multifamily Pipeline")
    if not dev_df.empty:
        st.dataframe(
            dev_df[["development_name", "municipality", "units", "status", "year_or_target_year"]].rename(
                columns={
                    "development_name": "Development",
                    "municipality": "Municipality",
                    "units": "Units",
                    "status": "Status",
                    "year_or_target_year": "Year"
                }
            ),
            hide_index=True,
            width="stretch",
            height=280,
        )
    else:
        st.info("No multifamily dataset found.")

    st.subheader("Squares per zone")
    zc = gdf.groupby("zone_name").size().rename("Squares").reset_index().rename(columns={"zone_name": "Zone"})
    st.dataframe(zc, hide_index=True, width="stretch")