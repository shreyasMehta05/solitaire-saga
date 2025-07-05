#version 330

uniform int gSelected;
uniform float gPulse;  // Pulse value for highlighting

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

void main()
{
    // Color palette based on suggestions
    vec3 marbleColor = vec3(0.12, 0.12, 0.12);         // #1E1E1E Dark gray/black default marble
    vec3 selectedColor = vec3(0.0, 0.78, 0.33);        // #00C853 Vibrant green for selected
    vec3 hoverColor = vec3(0.16, 0.71, 0.96);          // #29B6F6 Light blue for hover
    vec3 validMoveColor = vec3(1.0, 0.84, 0.0);        // #FFD600 Bright yellow for valid move
    
    // Choose color based on selection state
    vec3 baseColor;
    if (gSelected == 1) {
        // Selected marble - add pulsing effect
        float pulseIntensity = sin(gPulse) * 0.25 + 0.1;
        baseColor = mix(selectedColor, vec3(1.0), pulseIntensity);
    } else if (gSelected == 2) {
        baseColor = hoverColor;
    } else if (gSelected == 3) {
        // Valid move target - add pulsing effect
        float pulseIntensity = sin(gPulse) * 0.25 + 0.1;
        baseColor = mix(validMoveColor, vec3(1.0), pulseIntensity);
    } else {
        baseColor = marbleColor;
    }
    
    // Calculate lighting
    // Distance from center for simple sphere mapping
    float distFromCenter = length(FragPos.xy);
    
    // Simple sphere normal calculation
    vec3 normal = normalize(Normal);
    
    // Lighting direction (from upper right)
    vec3 lightDir = normalize(vec3(0.5, 0.5, 0.7));
    
    // Calculate diffuse lighting with warmer tint
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 warmDiffuse = vec3(1.0, 0.95, 0.9); // Slightly warm diffuse light
    vec3 diffuse = diff * warmDiffuse;
    
    // Calculate specular highlights with golden tint
    vec3 viewDir = vec3(0.0, 0.0, 1.0);  // Viewing from straight ahead
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);  // Sharper highlight
    vec3 specular = spec * vec3(0.95, 0.77, 0.06) * 0.9;  // #F1C40F Golden specular
    
    // Soft ambient light
    vec3 ambient = vec3(0.38, 0.38, 0.38); // #616161 Neutral gray
    
    // Add radial gradient effect to marble
    float gradient = 0.85 + 0.15 * (0.5 - distFromCenter);
    baseColor *= gradient;
    
    // Combine lighting with base color
    vec3 finalColor = baseColor * (ambient + diffuse) + specular;
    
    // Add gentle rim lighting
    float rim = 1.0 - max(dot(viewDir, normal), 0.0);
    rim = smoothstep(0.5, 0.85, rim);
    
    // Enhance selected marbles with stronger effects
    float alpha = 1.0;
    if (gSelected > 0) {
        // Add extra glow and rim lighting for selected marbles
        finalColor += baseColor * 0.2;
        finalColor += rim * 0.4 * baseColor;
        
        // Add a gentle outline glow
        float edge = smoothstep(0.7, 0.95, distFromCenter);
        finalColor = mix(finalColor, vec3(1.0) * baseColor, edge * 0.45);
    } else {
        // Add subtle rim lighting for regular marbles
        finalColor += rim * 0.15 * baseColor;
    }
    
    // Add a soft shadow at the bottom for depth
    float shadowFactor = max(0.0, FragPos.y * 0.5 + 0.2);
    finalColor *= shadowFactor;
    
    // Output with full opacity
    FragColor = vec4(finalColor, alpha);
}
