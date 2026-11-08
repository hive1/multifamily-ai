import streamlit as st
import geopandas as gpd
import pandas as pd
import folium
from streamlit_folium import st_folium
import numpy as np

# Set up page
st.set_page_config(layout="wide")
st.title("🗺️ Broome County Multifamily Availability Explorer (Beta)")
st.markdown("Interactive attribute mapping by US Census Tract boundaries.")

# 1. Fetch Geometries dynamically from US Census Bureau (FIPS 36 = NY, 007 = Broome)
@st.cache_data
def load_broome_geojson():
    url = "https://clst.org" # Alternative global base, or official Census API:
    # Direct reliable URL to NY Census Tracts from Census TIGER/Line
    census_url = "https://census.gov"
    
    # Read the zipped shapefile directly into Geopandas
    ny_tracts = gpd.read_file(census_url)
    # Filter for Broome County (COUNTYFP = '007')
    broome_tracts = ny_tracts[ny_tracts['COUNTYFP'] == '007'].copy()
    # Convert to standard lat/lon projection
    broome_tracts = broome_tracts.to_crs(epsg=4326)
    return broome_tracts

try:
    with st.spinner("Downloading Broome County Census boundaries..."):
        gdf = load_broome_geojson()
except Exception as e:
    st.error(f"Could not load census data: {e}")
    st.stop()

# 2. Generate or Load Your Specific Attributes
# Mocking attributes for demonstration: Median Income, Population Density, Poverty Rate
@st.cache_data
def get_mock_attributes(tract_ids):
    np.random.seed(42)
    data = {
        'GEOID': tract_ids,
        'Median Income ($)': np.random.randint(35000, 95000, size=len(tract_ids)),
        'Poverty Rate (%)': np.random.uniform(5.0, 35.0, size=len(tract_ids)).round(1),
        'Population Density (sq mi)': np.random.randint(100, 4000, size=len(tract_ids))
    }
    return pd.DataFrame(data)

df_attributes = get_mock_attributes(gdf['GEOID'].tolist())

# Merge geometries with attributes
merged_gdf = gdf.merge(df_attributes, on='GEOID')

# 3. Sidebar Configuration controls
st.sidebar.header("Map Configurations")
attribute_to_map = st.sidebar.selectbox(
    "Select Attribute to Visualize:",
    options=['Median Income ($)', 'Poverty Rate (%)', 'Population Density (sq mi)']
)

colormap_choice = st.sidebar.selectbox(
    "Color Theme:",
    options=['YlOrRd', 'Viridis', 'Blues', 'Plasma']
)

# 4. Render Interactive Folium Map
# Center around Binghamton / Broome County
map_center = [42.159, -75.933] 

m = folium.Map(location=map_center, zoom_start=10, tiles="cartodbpositron")

# Create Choropleth layer
choropleth = folium.Choropleth(
    geo_data=merged_gdf,
    name="choropleth",
    data=merged_gdf,
    columns=["GEOID", attribute_to_map],
    key_on="feature.properties.GEOID",
    fill_color=colormap_choice,
    fill_opacity=0.7,
    line_opacity=0.2,
    legend_name=attribute_to_map,
    highlight=True
).add_to(m)

# Add interactive tooltips on hover
style_function = lambda x: {'fillColor': '#ffffff', 'color':'#000000', 'fillOpacity': 0.1, 'weight': 0.1}
highlight_function = lambda x: {'fillColor': '#000000', 'color':'#000000', 'fillOpacity': 0.50, 'weight': 0.1}

# Tooltip config
folium.features.GeoJson(
    merged_gdf,
    style_function=style_function,
    control=False,
    highlight_function=highlight_function,
    tooltip=folium.features.GeoJsonTooltip(
        fields=['NAMELSAD', 'GEOID', 'Median Income ($)', 'Poverty Rate (%)'],
        aliases=['Tract Name:', 'GEOID Code:', 'Est. Median Income:', 'Poverty Rate:'],
        style=("background-color: white; color: #333333; font-family: arial; font-size: 12px; padding: 10px;")
    )
).add_to(m)

# Render map in Streamlit
col1, col2 = st.columns([3, 1])

with col1:
    st_data = st_folium(m, width=800, height=600, returned_objects=[])

with col2:
    st.subheader("Data Overview")
    st.write(f"Showing metrics for **{len(merged_gdf)}** unique tracts.")
    st.dataframe(
        merged_gdf[['NAMELSAD', attribute_to_map]]
        .sort_values(by=attribute_to_map, ascending=False),
        hide_index=True,
        use_container_width=True
    )
