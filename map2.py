from pathlib import Path

import branca.colormap as bcm
import folium
import geopandas as gpd
import numpy as np
import pandas as pd
import streamlit as st
from streamlit_folium import st_folium

st.set_page_config(layout="wide", page_title="Binghamton Census Tract Explorer")
st.title("🗺️ Binghamton Census Tract Explorer")
st.markdown("Explore housing need, rent burden, and multifamily housing developments across Broome County census tracts.")

# Paths
CENSUS_CSV = Path(__file__).with_name("census.csv")
DEV_FILE = Path(__file__).with_name("broome_multifamily_developments_expanded(1).csv")

# Census TIGER/Line tract polygons for New York (state 36, county 007 = Broome)
TRACT_URL = "https://www2.census.gov/geo/tiger/TIGER2023/TRACT/tl_2023_36_tract.zip"
UTM_18N = 32618  # Broome County UTM zone

LABELS = {
    "Total Population": "Total Population",
    ">30% Income is Considered Rent Burdened": "Rent Burdened Households",
    "Pop Employed": "Employed Population",
    "MedianRent": "Median Rent ($)",
}
NUMERIC = list(LABELS.keys())


@st.cache_data(show_spinner=True)
def load_census_data():
    df = pd.read_csv(CENSUS_CSV)
    # Ensure GEO_ID matches 11-digit string format (e.g., 1400000US36007000100 -> 36007000100)
    df["GEOID"] = df["GEO_ID"].astype(str).str.split("US").str[-1]
    return df


@st.cache_data(show_spinner=True)
def load_tract_boundaries():
    gdf = gpd.read_file(TRACT_URL)
    # Filter for Broome County (COUNTYFP == '007')
    broome = gdf[gdf["COUNTYFP"] == "007"].copy()
    broome = broome.to_crs(UTM_18N)
    return broome


@st.cache_data(show_spinner=False)
def load_developments():
    if DEV_FILE.exists():
        return pd.read_csv(DEV_FILE)
    return pd.DataFrame()


# Load dataframes
census_df = load_census_data()
tracts_gdf = load_tract_boundaries()
dev_df = load_developments()

# Merge spatial polygons with census statistics
merged_gdf = tracts_gdf.merge(census_df, left_on="GEOID", right_on="GEOID", how="left")
merged_gdf = merged_gdf.to_crs(4326)

# ---------------------------------------------------------------------------
# Sidebar Controls
# ---------------------------------------------------------------------------
st.sidebar.header("Map Configurations")
attr = st.sidebar.selectbox("Attribute to visualize:", list(LABELS), format_func=LABELS.get)
cmap_name = st.sidebar.selectbox(
    "Color theme:", ["YlOrRd_09", "Blues_09", "Purples_09", "Viridis", "Plasma"]
)
clip = st.sidebar.checkbox("Clip outliers (2nd-98th percentile)", value=True)
basemap = st.sidebar.radio("Base map:", ["Street", "Satellite"], horizontal=True)
show_developments = st.sidebar.checkbox("Show Multifamily Developments", value=True)

# ---------------------------------------------------------------------------
# Summary Metrics
# ---------------------------------------------------------------------------
m1, m2, m3, m4 = st.columns(4)
m1.metric("Census Tracts", f"{len(merged_gdf):,}")
m2.metric("Total Population", f"{census_df['Total Population'].sum():,.0f}")
m3.metric("Rent Burdened Households", f"{census_df['>30% Income is Considered Rent Burdened'].sum():,.0f}")
m4.metric("Multifamily Units (Pipeline)", f"{dev_df['units'].sum():,.0f}" if not dev_df.empty else "N/A")

# ---------------------------------------------------------------------------
# Color Scale
# ---------------------------------------------------------------------------
vals = merged_gdf[attr].fillna(0)
lo, hi = (np.percentile(vals, [2, 98]) if clip else (vals.min(), vals.max()))
if hi <= lo:
    hi = lo + 1

cmap = getattr(bcm.linear, cmap_name).scale(lo, hi)
cmap.caption = LABELS[attr]


def color_for(value):
    if pd.isna(value):
        return "#cccccc"
    return cmap(min(max(value, lo), hi))


# ---------------------------------------------------------------------------
# Build Map
# ---------------------------------------------------------------------------
minx, miny, maxx, maxy = merged_gdf.total_bounds
m = folium.Map(location=[(miny + maxy) / 2, (minx + maxx) / 2], zoom_start=11, tiles=None, max_zoom=22)

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

fields = ["GEOID", "NAME_y", "Total Population", ">30% Income is Considered Rent Burdened", "MedianRent"]
aliases = ["GEOID:", "Tract Name:", "Population:", "Rent Burdened:", "Median Rent ($):"]

folium.GeoJson(
    merged_gdf,
    name="Census Tracts",
    style_function=lambda f: {
        "fillColor": color_for(f["properties"].get(attr)),
        "color": "#333333",
        "weight": 1.0,
        "fillOpacity": 0.7,
    },
    highlight_function=lambda f: {"color": "#000000", "weight": 2.5, "fillOpacity": 0.9},
    tooltip=folium.GeoJsonTooltip(
        fields=["GEOID", "NAME_y", attr],
        aliases=["GEOID:", "Tract:", LABELS[attr] + ":"],
        style="background-color: white; color: #333; font-family: arial; font-size: 12px; padding: 8px;",
    ),
    popup=folium.GeoJsonPopup(fields=fields, aliases=aliases, max_width=320),
).add_to(m)

# Multifamily Developments Overlay
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
                radius=max(5, min(14, np.sqrt(row.get('units', 50)))),
                color="#1f77b4",
                fill=True,
                fill_color="#ff7f0e",
                fill_opacity=0.85,
                weight=2,
                popup=folium.Popup(popup_html, max_width=300),
                tooltip=f"{row.get('development_name')} ({row.get('units')} units)"
            ).add_to(dev_group)
    dev_group.add_to(m)

m.fit_bounds([[miny, minx], [maxy, maxx]], max_zoom=21)
cmap.add_to(m)
folium.LayerControl().add_to(m)

# ---------------------------------------------------------------------------
# Layout: Map + Details Panel
# ---------------------------------------------------------------------------
col_map, col_info = st.columns([3, 2])

with col_map:
    map_state = st_folium(
        m,
        height=640,
        use_container_width=True,
        key="tract_map",
        returned_objects=["last_active_drawing"],
    )

with col_info:
    clicked = (map_state or {}).get("last_active_drawing")
    selected_geoid = (clicked.get("properties") or {}).get("GEOID") if clicked else None

    if selected_geoid and (merged_gdf["GEOID"] == selected_geoid).any():
        row = merged_gdf[merged_gdf["GEOID"] == selected_geoid].iloc[0]
        st.subheader(f"Tract: {row['NAME_y']}")
        st.caption(f"GEOID: {selected_geoid}")

        a, b = st.columns(2)
        a.metric("Total Population", f"{row['Total Population']:,.0f}")
        b.metric("Rent Burdened HH", f"{row['>30% Income is Considered Rent Burdened']:,.0f}")
        a.metric("Employed Population", f"{row['Pop Employed']:,.0f}")
        b.metric("Median Rent", f"${row['MedianRent']:,.0f}" if pd.notna(row['MedianRent']) else "N/A")

        rank = int(merged_gdf[attr].rank(ascending=False, method="min")[row.name])
        st.write(f"Ranks **#{rank} of {len(merged_gdf)}** tracts by {LABELS[attr].lower()}.")
        st.divider()
    else:
        st.info("Click a census tract on the map to inspect its demographic and housing breakdown.")

    st.subheader("Broome County Multifamily Pipeline")
    if not dev_df.empty:
        st.dataframe(
            dev_df[["development_name", "municipality", "units", "status"]].rename(
                columns={
                    "development_name": "Development",
                    "municipality": "Municipality",
                    "units": "Units",
                    "status": "Status",
                }
            ),
            hide_index=True,
            width="stretch",
            height=240,
        )

    st.subheader("All Census Tracts")
    table = merged_gdf[["GEOID", "NAME_y"] + NUMERIC].rename(
        columns={"GEOID": "GEOID", "NAME_y": "Tract Name", **LABELS}
    )
    st.dataframe(
        table.sort_values(LABELS[attr], ascending=False),
        hide_index=True,
        width="stretch",
        height=280,
    )
