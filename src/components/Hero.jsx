import React from 'react';
import { Box, Typography, Button, Container } from '@mui/material';

function Hero() {
  return (
    <Box id="home" sx={{ bgcolor: '#0a1929', color: 'white', py: 12 }}>
      <Container maxWidth="md" sx={{ textAlign: 'center' }}>
        <Typography variant="h2" component="h1" gutterBottom sx={{ fontWeight: 'bold' }}>
            Move faster with <Box component="span" sx={{ color: 'hsl(210, 100%, 60%)' }}>HERMES</Box>
        </Typography>
        <Typography variant="h6" paragraph>
          Download .deb installers for Selective Repeat, Go-Back-N, and Stop & Wait protocols instantly.
        </Typography>
        <Button
          variant="contained"
          size="large"
          href="#download"
          sx={{ textTransform: 'none', mt: 3 }}
        >
          Get Started
        </Button>
      </Container>
    </Box>
  );
}

export default Hero;